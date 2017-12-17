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


#include "lib/globals.hpp"
#include "lib/config.hpp"
#include "lib/exceptions.hpp"

#include "lib/threads/watchdog_thread.hpp"
#include "lib/logger.hpp"
#include "lib/string_lib.hpp"
#include "lib/threads/watchdog_thread.hpp"
#include <pthread.h>

#include <signal.h>
#include <unistd.h>
#include <string.h>

#include  "lib/threads/thread_registry.hpp"

#include <iostream>

DEF_LOGGER_STAT( "BBB_HVAC::GLOBALS" );

using namespace BBB_HVAC;

namespace BBB_HVAC
{
	namespace GLOBALS
	{
		bool global_exit_flag = false;

		WATCHDOG* watchdog;

		LOGIC_PROCESSOR_BASE* logic_instance = nullptr;

		LOGGING::LOG_CONFIGURATOR* root_log_configurator = nullptr;

		//IOCOMM::SER_IO_COMM * io_instance = nullptr;
	}
}

using namespace BBB_HVAC;

namespace BBB_HVAC
{
	namespace GLOBALS
	{

		string sig_to_str( int _sig )
		{
			if( _sig == SIGHUP )
			{
				return "SIGHUP";
			}

			if( _sig == SIGINT )
			{
				return "SIGINT";
			}

			if( _sig == SIGQUIT )
			{
				return "SIGQUIT";
			}

			if( _sig == SIGILL )
			{
				return "SIGILL";
			}

			if( _sig == SIGTRAP )
			{
				return "SIGTRAP";
			}

			if( _sig == SIGABRT )
			{
				return "SIGABRT";
			}

			if( _sig == SIGIOT )
			{
				return "SIGIOT";
			}

			if( _sig == SIGBUS )
			{
				return "SIGBUS";
			}

			if( _sig == SIGFPE )
			{
				return "SIGFPE";
			}

			if( _sig == SIGKILL )
			{
				return "SIGKILL";
			}

			if( _sig == SIGUSR1 )
			{
				return "SIGUSR1";
			}

			if( _sig == SIGSEGV )
			{
				return "SIGSEGV";
			}

			if( _sig == SIGUSR2 )
			{
				return "SIGUSR2";
			}

			if( _sig == SIGPIPE )
			{
				return "SIGPIPE";
			}

			if( _sig == SIGALRM )
			{
				return "SIGALRM";
			}

			if( _sig == SIGTERM )
			{
				return "SIGTERM";
			}

			if( _sig == SIGSTKFLT )
			{
				return "SIGSTKFLT";
			}

			if( _sig == SIGCLD )
			{
				return "SIGCLD";
			}

			if( _sig == SIGCHLD )
			{
				return "SIGCHLD";
			}

			if( _sig == SIGCONT )
			{
				return "SIGCONT";
			}

			if( _sig == SIGSTOP )
			{
				return "SIGSTOP";
			}

			if( _sig == SIGTSTP )
			{
				return "SIGTSTP";
			}

			if( _sig == SIGTTIN )
			{
				return "SIGTTIN";
			}

			if( _sig == SIGTTOU )
			{
				return "SIGTTOU";
			}

			if( _sig == SIGURG )
			{
				return "SIGURG";
			}

			if( _sig == SIGXCPU )
			{
				return "SIGXCPU";
			}

			if( _sig == SIGXFSZ )
			{
				return "SIGXFSZ";
			}

			if( _sig == SIGVTALRM )
			{
				return "SIGVTALRM";
			}

			if( _sig == SIGPROF )
			{
				return "SIGPROF";
			}

			if( _sig == SIGWINCH )
			{
				return "SIGWINCH";
			}

			if( _sig == SIGPOLL )
			{
				return "SIGPOLL";
			}

			if( _sig == SIGIO )
			{
				return "SIGIO";
			}

			if( _sig == SIGPWR )
			{
				return "SIGPWR";
			}

			if( _sig == SIGSYS )
			{
				return "SIGSYS";
			}

			if( _sig == SIGUNUSED )
			{
				return "SIGUNUSED";
			}

			return string( "UNKNOWN(" ) + num_to_str( _sig ) + string( ")" );
		}

		void configure_logging( const LOGGING::ENUM_LOG_LEVEL& _level )
		{
			root_log_configurator = new LOGGING::LOG_CONFIGURATOR( _level );
		}

		extern void destroy_watchdog()
		{
			if( watchdog != nullptr )
			{
				watchdog->stop_thread();
			}

			return;
		}

		void configure_watchdog( void )
		{
			if( watchdog == nullptr )
			{
				watchdog = new WATCHDOG();
				watchdog->start_thread();
			}
		}

		static bool in_trap = false;

		void signal_trap( int sig )
		{
			/*
			 * This seems like a bad idea
			 */
			if( in_trap == true )
			{
				return;
			}

			LOG_INFO_STAT( string( "Received signal: " ) + sig_to_str( sig ) + " (" + num_to_str( sig ) + ")" );
			LOG_DEBUG_STAT( string( "********************************************************************" ) );
			in_trap = true;
			THREAD_REGISTRY::stop_all();
			LOG_DEBUG_STAT( string( "Cleanup finished.  Exiting." ) );
			GLOBALS::global_exit_flag = true;
			return;
		}

		void configure_signals( void )
		{
			struct sigaction sa;
			memset( &sa, 0, sizeof( struct sigaction ) );
			sa.sa_handler = signal_trap;

			if( sigaction( SIGINT, &sa, nullptr ) != 0 ) //2
			{
				LOG_ERROR_STAT( create_perror_string( "Failed to install signal handler for SIGINT" ) );
			}

			/*
			 if (sigaction(SIGKILL, &sa, nullptr) != 0)
			 {
			 LOG_ERROR_STAT(create_perror_string("Failed to install signal handler for SIGKILL"));
			 }*/

			if( signal( SIGPIPE, SIG_IGN ) != 0 )
			{
				LOG_ERROR_STAT( create_perror_string( "Failed to ignore SIGPIPE" ) );
			}

			if( sigaction( SIGABRT, &sa, nullptr ) != 0 ) //6
			{
				LOG_ERROR_STAT( create_perror_string( "Failed to install signal handler for SIGABRT" ) );
			}

			if( sigaction( SIGFPE, &sa, nullptr ) != 0 ) //8
			{
				LOG_ERROR_STAT( create_perror_string( "Failed to install signal handler for SIGFPE" ) );
			}

			if( sigaction( SIGILL, &sa, nullptr ) != 0 ) //4
			{
				LOG_ERROR_STAT( create_perror_string( "Failed to install signal handler for SIGILL" ) );
			}

			if( sigaction( SIGSEGV, &sa, nullptr ) != 0 )
			{
				LOG_ERROR_STAT( create_perror_string( "Failed to install signal handler for SIGSEGV" ) );
			}

			if( sigaction( SIGTERM, &sa, nullptr ) != 0 )
			{
				LOG_ERROR_STAT( create_perror_string( "Failed to install signal handler for SIGTERM" ) );
			}

			if( sigaction( SIGHUP, &sa, nullptr ) != 0 )
			{
				LOG_ERROR_STAT( create_perror_string( "Failed to install signal handler  for SIGHUP" ) );
			}

			return;
		}
	}
}
