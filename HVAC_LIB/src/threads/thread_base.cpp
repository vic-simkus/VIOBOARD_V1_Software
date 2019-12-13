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

#include "lib/threads/thread_base.hpp"
#include "lib/threads/thread_registry.hpp"
#include "lib/logger.hpp"

#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

using namespace BBB_HVAC;

THREAD_BASE::THREAD_BASE( const string& _tag ) : TPROTECT_BASE( _tag )
{
	this->abort_thread = false;
	this->is_running = false;
	this->thread_tag = _tag;
	this->do_not_self_delete = false;
	this->is_io_thread = false;
	this->logger = new LOGGING::LOGGER( "BBB_HVAC::THREAD_BASE[" + this->thread_tag + "]" );
}

THREAD_BASE::~THREAD_BASE()
{
	delete this->logger;
	this->logger = nullptr;
	return;
}

TPROTECT_BASE* THREAD_BASE::obtain_lock( void ) throw( LOCK_ERROR )
{
	try
	{
		this->obtain_lock_ex( &this->abort_thread );
	}
	catch ( const LOCK_ERROR& _ex )
	{
		LOG_ERROR_P( "Failed to obtain lock: " + string( _ex.what() ) );
		this->abort_thread = true;
		THROW_EXCEPTION( LOCK_ERROR, "Failed to obtain lock in thread [" + this->thread_tag + "]" );
	}

	return this;
}

void THREAD_BASE::pthread_func( void )
{
	pthread_detach( pthread_self() );
	this->is_running = true;

	LOG_DEBUG_P( "Thread started.  TID: " + num_to_str( gettid() ) );

	if ( this->is_io_thread )
	{
		THREAD_REGISTRY::register_thread( this, THREAD_TYPES_ENUM::IO_THREAD );
	}
	else
	{
		THREAD_REGISTRY::register_thread( this );
	}

	try
	{
		if ( !this->thread_func() )
		{
			LOG_ERROR_P( "thread_func for [" + this->thread_tag + "] returned false." );
		}
		else
		{
			LOG_DEBUG_P( "thread_func for [" + this->thread_tag + "] returned true." );
		}
	}
	catch ( const exception& _e )
	{
		LOG_ERROR_P( "Caught an unhandled exception: " + string( _e.what() ) );
		LOG_ERROR_P( "Aborting." );
	}

	if ( !this->do_not_self_delete )
	{
		THREAD_REGISTRY::delete_thread( this );
	}

	this->is_running = false;
	LOG_DEBUG_P( "Thread stopped." )
	pthread_exit( nullptr );
	return;
}

void THREAD_BASE::start_thread( void )
{
	pthread_create( &this->root_tid, nullptr, ( void* ( * )( void* ) ) thread_base_shim_func, this );
	return;
}

string THREAD_BASE::get_thread_tag( void ) const
{
	return this->thread_tag;
}

void THREAD_BASE::stop_thread( bool _self_delete )
{
	LOG_DEBUG_P( "Attempting to stop" );

	if ( this->is_running == false )
	{
		LOG_DEBUG_P( "Thread is already dead." );
		return;
	}

	this->do_not_self_delete = true;
	this->abort_thread = true;
	timespec st;

	int attempt_count = 0;

	while ( this->is_running )
	{
		st.tv_sec = 0;
		st.tv_nsec = 10000000; //10 milisecond
		nsleep( &st );

		attempt_count += 1;

		if ( attempt_count >= 100 )
		{
			LOG_ERROR_P( "Thread [" + num_to_str( gettid() ) + "] is failing to die gracefully." );
			this->is_running = false;
			break;
		}
	}

	if ( _self_delete )
	{
		THREAD_REGISTRY::delete_thread( this );
	}

	LOG_DEBUG_P( "Has been stopped." );
	return;
}