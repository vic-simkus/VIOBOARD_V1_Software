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


#include "lib/tprotect_base.hpp"
#include "lib/logger.hpp"
#include "lib/exceptions.hpp"
#include "lib/config.hpp"

#include <string.h>

using namespace BBB_HVAC;

TPROTECT_BASE::TPROTECT_BASE( const string& _tag )
{
	this->tag = _tag;
	this->rand_seed = ( unsigned int ) time( nullptr );
	pthread_mutexattr_t mutex_attr;

	if ( pthread_mutexattr_init( &mutex_attr ) != 0 )
	{
		throw runtime_error( create_perror_string( this->tag + ": Failed to initialize mutex attribute." ) );
	}

	pthread_mutexattr_settype( &mutex_attr, PTHREAD_MUTEX_ERRORCHECK );

	if ( pthread_mutex_init( & ( this->mutex ), &mutex_attr ) != 0 )
	{
		throw runtime_error( create_perror_string( this->tag + ": Failed to initialize mutex." ) );
	}

	this->reset_sleep_timespec();
	INIT_LOGGER( "BBB_HVAC::TPROTECT_BASE[" + this->tag + "]" );
	return;
}

void TPROTECT_BASE::reset_sleep_timespec( ssize_t _time )
{
	memset( & ( this->thread_sleep ), 0, sizeof( struct timespec ) );

	if ( _time > 0 )
	{
		this->thread_sleep.tv_nsec = _time;
	}

	return;
}

TPROTECT_BASE::~TPROTECT_BASE()
{
	if ( pthread_mutex_destroy( & ( this->mutex ) ) != 0 )
	{
		//LOG_ERROR_P( create_perror_string( this->tag + ": Failed to destroy mutex" ) );
		//XXX - Can't use the logger anymore?
	}

	this->tag.clear();
	return;
}

void TPROTECT_BASE::nsleep( timespec* _time ) const throw( runtime_error )
{
	timespec sleep_time;
	timespec thread_sleep_rem;

	if ( _time == nullptr )
	{
		memcpy( &sleep_time, &this->thread_sleep, sizeof( struct timespec ) );
	}
	else
	{
		memcpy( &sleep_time, _time, sizeof( struct timespec ) );
	}

	int rc = nanosleep( &sleep_time, &thread_sleep_rem );

	if ( rc != 0 )
	{
		/*
		 * Some man pages state that EINTR will be returned in case of interruption.
		 * Other man pages state that -1 will be returned and errno will be set to EINTR.
		 * Who the fuck knows.
		 */
		if ( rc == EINTR || ( rc == -1 && errno == EINTR ) )
		{
			/*
			 * Sleep was interrupted.  Go back to sleep for remaining time.
			 */
			nanosleep( &thread_sleep_rem, nullptr );
		}
		else if ( rc == -1 )
		{
			THROW_EXCEPTION( runtime_error, create_perror_string( this->tag + ": NANOSLEEP failed with code -1: " ) );
		}
		else
		{
			THROW_EXCEPTION( runtime_error, this->tag + ": NANOSLEEP failed with code: " + num_to_str( rc ) );
		}
	}

	return;
}

bool TPROTECT_BASE::obtain_lock_ex( void )  throw( LOCK_ERROR )
{
	bool cond = false;
	return this->obtain_lock_ex( &cond );
}

bool TPROTECT_BASE::obtain_lock_ex( const bool* _cond ) throw( LOCK_ERROR )
{
	int mutex_lock_result = 0;
	unsigned int mutex_lock_attempts = 0;
	timespec sleep_tv;

	while ( *_cond == false )
	{
		if ( mutex_lock_attempts > 0 )
		{
			/*
			 * If we're spinning waiting for a mutex add jitter to the sleep amount so that we don't accidentally synchronize with some other thread.
			 */
			memset( &sleep_tv, 0, sizeof( struct timespec ) );
			sleep_tv.tv_nsec += rand_r( &this->rand_seed );

			if ( sleep_tv.tv_nsec == 0 )
			{
				sleep_tv.tv_nsec = 10000; // 10 uSec
			}

			/*
			 * We want the minimum sleep to me 1uSec
			 */
			sleep_tv.tv_nsec = sleep_tv.tv_nsec | 0x3E8;
			/*
			 * But we don't want to go over 1 millisecond
			 */
			sleep_tv.tv_nsec = sleep_tv.tv_nsec & 0xF4240;
			this->nsleep( &sleep_tv );
		}

		mutex_lock_result = pthread_mutex_trylock( &this->mutex );

		if ( mutex_lock_result != 0 )
		{
			if ( mutex_lock_result == EBUSY )
			{
				/*
				 * Failed to acquire a the main mutex.
				 */
				if ( mutex_lock_attempts == GC_MUTEX_LOCK_ATTEMPT )
				{
					/*
					 * If we've tried a number of times to obtain the mutex and failed bail
					 * because something has obviously gone wrong.
					 *
					 * TODO - need to handle this better.  One misbehaving client can take the whole LOGIC_CORE down by hogging a mutex lock.
					 * If we fail to obtain a lock in a LOGIC_CORE thread we need to start shedding client connections before aborting the whole process.
					 */
					THROW_EXCEPTION( LOCK_ERROR, this->tag + ": Failed to obtain mutex lock." );
				}
				else
				{
					/*
					 * We failed to ge the mutex, but have not yet run out of attempts.
					 */
					mutex_lock_attempts += 1;
					continue;
				}
			}
			else
			{
				THROW_EXCEPTION( LOCK_ERROR, this->tag + ": Unexpected value returned from pthread_mutex_trylock: " + num_to_str( mutex_lock_result ) + "." );
			}
		}
		else
		{
			/*
			 * We obtained the mutex.
			 */
			return ( true );
		}
	}

	THROW_EXCEPTION( LOCK_ERROR, this->tag + ": Lock acquisition loop aborted on condition." );
}

bool TPROTECT_BASE::release_lock() throw( LOCK_ERROR )
{
	int rc = 0;

	if ( ( rc = pthread_mutex_unlock( & ( this->mutex ) ) ) != 0 )
	{
		THROW_EXCEPTION( LOCK_ERROR, this->tag + ": Failed to release lock." );
	}

	return true;
}

