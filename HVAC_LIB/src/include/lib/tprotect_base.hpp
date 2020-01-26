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


#ifndef SRC_INCLUDE_LIB_TPROTECT_BASE_HPP_
#define SRC_INCLUDE_LIB_TPROTECT_BASE_HPP_

#include <time.h>
#include <pthread.h>
#include <string>

#include "lib/exceptions.hpp"
#include "logger.hpp"

using namespace std;

namespace BBB_HVAC
{
	using namespace EXCEPTIONS;


	/**
	 *\brief A class that provides a mutex and lock/unlock method with retries, timeouts, etc.  It is intended to be a base class for any class
	 * that needs to have synchronization primitives.
	 */
	class TPROTECT_BASE
	{
		public:
			/**
			 * \brief Constructor.
			 * \param _tag Identification tag.  Used in log output to aid in troubleshooting
			 */
			TPROTECT_BASE( const string& _tag );
			/**
			 * \brief Destructor.
			 */
			virtual ~TPROTECT_BASE();

			/**
			 * \brief Attempts to obtain a lock on the mutex.
			 * Under the hood it calls obtain_lock_ex with a pointer to a local 'false' condition.
			 * \return Always returns true.  If something goes wonky and exception is thrown.
			 * \see obtain_lock_ex(const bool*)
			 */
			bool obtain_lock_ex( void ) throw( LOCK_ERROR );
			/**
			 * \brief Attempts to obtain a lock on the mutex.
			 * If an initial attempt fails, it tries GC_MUTEX_LOCK_ATTEMPT times to obtain a lock with an rand_r nanoseonds
			 * between attempts.  If after all that it still can't obtain a lock an exception is thrown.
			 * All attempts are predicated on _cond being false.  If _cond becomes true the attemp loop aborts with an exception.
			 * \return Always returns true.  If something goes wonky an exception is thrown.
			 */
			bool obtain_lock_ex( const bool* _cond ) throw( LOCK_ERROR );

			/**
			 * \brief Relieses lock obtained by obtain_lock
			 * \return Always returns true.  If something goes wonky an exception is thrown.
			 * \see obtain_lock(void)
			 */
			bool release_lock( void ) throw( LOCK_ERROR );

			/**
			 * \brief Sleeps for a specified time.  The method will make a best faith effort to sleep for the full specified period.
			 * \param _time Sleep period.  If nullptr is supplied this->thread_sleep is used.
			 */
			void nsleep( timespec* _time = nullptr ) const throw( runtime_error );

			/**
			 * \brief Resets this->thread_sleep to the specified number of nanoseconds.
			 * \param _nsecs Number of nanoseconds to initialize the struct to to.  If -1 is passed the struct is zeroed out.
			 */
			void reset_sleep_timespec( ssize_t _nsecs = -1 );

		protected:

			/**
			 * \brief Mutex protecting the instance.
			 */
			pthread_mutex_t mutex;

		private:
			/**
			 * \brief Logger instance.
			 */
			DEF_LOGGER;

			/**
			 * Instance tag.  Used for log output for debugging.
			 */
			string tag;

			/**
			 * \brief Random seed.  Initialized at class instantiation.  Used to generate random sleep intervals when trying and failing to obtain a lock.
			 */
			unsigned int rand_seed;

			/**
			 * \brief Struct used in sleeping.
			 */
			timespec thread_sleep;

	} ;
}

#endif /* SRC_INCLUDE_LIB_TPROTECT_BASE_HPP_ */
