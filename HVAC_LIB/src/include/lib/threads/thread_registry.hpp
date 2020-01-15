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

#ifndef THREAD_REGISTRY_HPP
#define THREAD_REGISTRY_HPP

#include "lib/tprotect_base.hpp"
#include "lib/logger.hpp"
#include "lib/exceptions.hpp"

#include "lib/threads/serial_io_thread.hpp"

#include <vector>



using std::vector;

namespace BBB_HVAC
{
	class THREAD_BASE;

	enum class THREAD_TYPES_ENUM : unsigned char
	{
		NONE,
		IO_THREAD
	} ;

	class THREAD_REGISTRY : public TPROTECT_BASE
	{
		public:
			~THREAD_REGISTRY();

			static void register_thread( THREAD_BASE* _thread, THREAD_TYPES_ENUM _type = THREAD_TYPES_ENUM::NONE ) throw( runtime_error );
			static void delete_thread( THREAD_BASE* _thread ) throw( runtime_error );
			static void stop_all( void ) throw( runtime_error );
			static void init_cleanup( void ) throw( runtime_error );
			static void destroy_global( void );

			static inline void register_io_death_listener( void ( *_ptr )( const std::string& ) ) {
				THREAD_REGISTRY::io_death_listener = _ptr;
				return;
			}

			static const vector<THREAD_BASE*>* get_io_threads( void ) throw( runtime_error );
			static IOCOMM::SER_IO_COMM* get_serial_io_thread( const std::string& _tag ) throw( runtime_error );

			static inline void global_cleanup( void ) {
				THREAD_REGISTRY::global_instance->cleanup();
			}

			static THREAD_REGISTRY* get_instance( void );

		protected:
			THREAD_REGISTRY( const string& _tag );


			void reg_thread( THREAD_BASE* _thread, THREAD_TYPES_ENUM _type = THREAD_TYPES_ENUM::NONE ) throw( runtime_error );
			void del_thread( THREAD_BASE* _thread, bool _lockl ) throw( runtime_error );
			void stop_all_threads( void ) throw( runtime_error );
			void cleanup( bool _lock = true ) throw( runtime_error );

			bool is_thread_active( const THREAD_BASE* _ptr ) const;

		private:

			void __cleanup( void ) throw( runtime_error );

			static void ( *io_death_listener )( const std::string& );
			DEF_LOGGER;

			vector<THREAD_BASE*> io_threads;
			vector<THREAD_BASE*> active_threads;
			vector<THREAD_BASE*> dead_threads;

			static THREAD_REGISTRY* global_instance;

			bool in_stop_all;
	} ;
}


#endif /* THREAD_REGISTRY_HPP */

