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

#ifndef THREAD_BASE_HPP
#define THREAD_BASE_HPP

#include <string>
using std::string;

#include <pthread.h>
#include <time.h>
#include "lib/exceptions.hpp"
#include "lib/tprotect_base.hpp"
#include "lib/logger.hpp"

namespace BBB_HVAC
{
	using namespace EXCEPTIONS;

	class THREAD_BASE : public TPROTECT_BASE
	{
		public:
			friend void thread_base_shim_func( void* );

			THREAD_BASE( const string& _tag );
			~THREAD_BASE();

			void start_thread( void );
			void stop_thread( bool _self_delete = false );

			inline void flag_for_stop( void ) {
				this->abort_thread = true;
			}

			string get_thread_tag( void ) const;

			void obtain_lock( void ) throw( LOCK_ERROR );

			inline bool get_is_io_thread( void ) const {
				return this->is_io_thread;
			}
		protected:

			void pthread_func( void );
			virtual bool thread_func( void ) = 0;
			string thread_tag;


			/**
			 * Flag to abort the processing thread.  When set to true the thread will abort during next iteration.
			 */
			bool abort_thread;
			bool is_running;
			bool do_not_self_delete;
			bool is_io_thread;

			DEF_LOGGER;
		private:


			pthread_t root_tid;

	} ;

	inline void thread_base_shim_func( void* _parm )
	{
		( ( THREAD_BASE* ) _parm )->pthread_func();
	}
}


#endif /* THREAD_BASE_HPP */

