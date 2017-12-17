/*
* This file is part of the software stack for Vic's IO board and its
* associated projects.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* Copyright 2016,2017,2018 Vidas Simkus (vic.simkus@gmail.com)
*/

#include "lib/threads/thread_registry.hpp"
#include "lib/threads/thread_base.hpp"

#include <algorithm>

using namespace BBB_HVAC;

THREAD_REGISTRY* THREAD_REGISTRY::global_instance = nullptr;

THREAD_REGISTRY::THREAD_REGISTRY( const string& _tag ) : TPROTECT_BASE( _tag )
{
	this->logger = new LOGGING::LOGGER();
	INIT_LOGGER_P( "BBB_HVAC::THREAD_REGISTRY [" + _tag + + "]" );
	this->in_stop_all = false;
	return;
}

THREAD_REGISTRY::~THREAD_REGISTRY()
{
	delete this->logger;
	this->logger = nullptr;
}

void THREAD_REGISTRY::register_thread( THREAD_BASE* _thread, THREAD_TYPES_ENUM _type ) throw( runtime_error )
{
	get_instance()->reg_thread( _thread, _type );
}

void THREAD_REGISTRY::delete_thread( THREAD_BASE* _thread ) throw( runtime_error )
{
	get_instance()->del_thread( _thread, true );
}

void THREAD_REGISTRY::stop_all( void ) throw( runtime_error )
{
	get_instance()->stop_all_threads();
}

void THREAD_REGISTRY::init_cleanup( void ) throw( runtime_error )
{
	get_instance()->cleanup( true );
}

THREAD_REGISTRY* THREAD_REGISTRY::get_instance( void )
{
	if( THREAD_REGISTRY::global_instance == nullptr )
	{
		THREAD_REGISTRY::global_instance = new THREAD_REGISTRY( "THREAD_REGISTRY" );
	}

	return THREAD_REGISTRY::global_instance;
}

bool THREAD_REGISTRY::is_thread_active( const THREAD_BASE* _ptr ) const
{
	for( vector<THREAD_BASE*>::const_iterator i = this->active_threads.begin(); i != this->active_threads.end(); ++i )
	{
		if( ( *i ) == _ptr )
		{
			return true;
		}
	}

	return false;
}

void THREAD_REGISTRY::reg_thread( THREAD_BASE* _thread, THREAD_TYPES_ENUM _type ) throw( runtime_error )
{
	if( this->in_stop_all )
	{
		LOG_ERROR_P( "Tried to register thread while in stop-all mode." );
		return;
	}

	this->obtain_lock_ex();

	if( this->is_thread_active( _thread ) )
	{
		this->release_lock();
		THROW_EXCEPTION( runtime_error, "Tried to register more than once a thread with the same instance pointer." );
	}

	this->active_threads.push_back( _thread );

	if( _type == THREAD_TYPES_ENUM::IO_THREAD )
	{
		this->io_threads.push_back( _thread );
	}

	this->release_lock();
	return;
}

void THREAD_REGISTRY::del_thread( THREAD_BASE* _thread, bool _lock ) throw( runtime_error )
{
	if( this->in_stop_all )
	{
		return;
	}

	LOG_DEBUG_P( "Deleting thread" );

	if( _lock )
	{
		this->obtain_lock_ex();
	}

	auto i  = std::find( std::begin( this->active_threads ), std::end( this->active_threads ), _thread );

	if( i == std::end( this->active_threads ) )
	{
		if( _lock )
		{
			this->release_lock();
		}

		THROW_EXCEPTION( runtime_error, "Tried to delete a thread instance that is not registered." );
	}

	/*
	 * Apparently in lala land that is the C++ standard library move does not mean move.  It means copy.
	 * Copy probably means move.
	 */
	std::move( i, i + 1, std::back_inserter( this->dead_threads ) );
	this->active_threads.erase( i );
	i  = std::find( std::begin( this->io_threads ), std::end( this->io_threads ), _thread );

	if( i != std::end( this->io_threads ) )
	{
		this->io_threads.erase( i );
	}

	if( _lock )
	{
		this->release_lock();
	}

	return;
}

void THREAD_REGISTRY::stop_all_threads( void ) throw( runtime_error )
{
	if( this->in_stop_all )
	{
		return;
	}

	this->obtain_lock_ex();
	this->in_stop_all = true;
	LOG_DEBUG_P( "Stopping all threads." );

	/*
	 * First loop through all the threads and flag them for stoppage.
	 * This will give them a chance to iterate through their event loops and cleanup.
	 */
	for( auto i = this->active_threads.begin(); i != this->active_threads.end(); ++i )
	{
		if( *i != nullptr )
		{
			( *i )->flag_for_stop();
		}
		else
		{
			LOG_ERROR_P( "NULLPTR in registry." );
		}
	}

	/*
	 * Here we make sure that each thread has indeed stopped.
	 * None of the threads will be moved to the dead thread queue because we set in_stop_app.
	 */
	for( auto i = this->active_threads.begin(); i != this->active_threads.end(); ++i )
	{
		if( *i != nullptr )
		{
			( *i )->stop_thread( false );
		}
		else
		{
			LOG_ERROR_P( "NULLPTR in registry." );
		}
	}

	std::move( std::begin( this->active_threads ), std::end( this->active_threads ), std::back_inserter( this->dead_threads ) );
	this->active_threads.clear();
	this->__cleanup();
	this->in_stop_all = false;
	this->release_lock();
}

void THREAD_REGISTRY::__cleanup( void ) throw( runtime_error )
{
	for( auto i = this->dead_threads.begin(); i != this->dead_threads.end(); ++i )
	{
		if( *i != nullptr )
		{
			delete( *i );
			( *i ) = nullptr;
		}
		else
		{
			LOG_ERROR_P( "NULLPTR in registry." );
		}
	}

	this->dead_threads.clear();
}

void THREAD_REGISTRY::cleanup( bool _lock ) throw( runtime_error )
{
	if( this->in_stop_all )
	{
		return;
	}

	if( _lock )
	{
		this->obtain_lock_ex();
	}

	this->__cleanup();

	if( _lock )
	{
		this->release_lock();
	}

	return;
}

void THREAD_REGISTRY::destroy_global( void )
{
	if( THREAD_REGISTRY::global_instance != nullptr )
	{
		delete THREAD_REGISTRY::global_instance;
		THREAD_REGISTRY::global_instance = nullptr;
	}
}


const vector<THREAD_BASE*>* THREAD_REGISTRY::get_io_threads( void )  throw( runtime_error )
{
	/*
	XXX
	So couple possible issues here.

	We're not taking locking into account here because the IO threads are expected to last for the lifespan of the process.
	We're not accounting for the thread instance going null if the thread deletes it self.  How can we?
	*/
	return &THREAD_REGISTRY::global_instance->io_threads;
}

IOCOMM::SER_IO_COMM* THREAD_REGISTRY::get_serial_io_thread( const std::string& _tag )  throw( runtime_error )
{
	for( size_t i = 0; i < THREAD_REGISTRY::global_instance->io_threads.size(); i++ )
	{
		THREAD_BASE* tb = THREAD_REGISTRY::global_instance->io_threads[i];
		IOCOMM::SER_IO_COMM* st = dynamic_cast<IOCOMM::SER_IO_COMM*>( tb );

		if( st == nullptr )
		{
			THROW_EXCEPTION( runtime_error, "Failed to cast pointer to SER_IO_COMM" );
		}

		if( st->get_tag() == _tag )
		{
			return st;
		}
	}

	THROW_EXCEPTION( runtime_error, "Failed to find serial IO thread instance with tag: " + _tag );
}
