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
#include "lib/log_configurator.hpp"

#include "lib/threads/watchdog_thread.hpp"
#include "lib/logger.hpp"
#include "lib/string_lib.hpp"
#include "lib/threads/watchdog_thread.hpp"
#include <pthread.h>

#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "lib/threads/thread_registry.hpp"
#include "lib/command_line_parms.h"

#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <fcntl.h>
#include <pwd.h>
#include <grp.h>



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
			if ( _sig == SIGHUP )
			{
				return "SIGHUP";
			}

			if ( _sig == SIGINT )
			{
				return "SIGINT";
			}

			if ( _sig == SIGQUIT )
			{
				return "SIGQUIT";
			}

			if ( _sig == SIGILL )
			{
				return "SIGILL";
			}

			if ( _sig == SIGTRAP )
			{
				return "SIGTRAP";
			}

			if ( _sig == SIGABRT )
			{
				return "SIGABRT";
			}

			if ( _sig == SIGIOT )
			{
				return "SIGIOT";
			}

			if ( _sig == SIGBUS )
			{
				return "SIGBUS";
			}

			if ( _sig == SIGFPE )
			{
				return "SIGFPE";
			}

			if ( _sig == SIGKILL )
			{
				return "SIGKILL";
			}

			if ( _sig == SIGUSR1 )
			{
				return "SIGUSR1";
			}

			if ( _sig == SIGSEGV )
			{
				return "SIGSEGV";
			}

			if ( _sig == SIGUSR2 )
			{
				return "SIGUSR2";
			}

			if ( _sig == SIGPIPE )
			{
				return "SIGPIPE";
			}

			if ( _sig == SIGALRM )
			{
				return "SIGALRM";
			}

			if ( _sig == SIGTERM )
			{
				return "SIGTERM";
			}

#ifndef __FreeBSD__

			if ( _sig == SIGSTKFLT )
			{
				return "SIGSTKFLT";
			}

			if ( _sig == SIGCLD )
			{
				return "SIGCLD";
			}

#endif

			if ( _sig == SIGCHLD )
			{
				return "SIGCHLD";
			}

			if ( _sig == SIGCONT )
			{
				return "SIGCONT";
			}

			if ( _sig == SIGSTOP )
			{
				return "SIGSTOP";
			}

			if ( _sig == SIGTSTP )
			{
				return "SIGTSTP";
			}

			if ( _sig == SIGTTIN )
			{
				return "SIGTTIN";
			}

			if ( _sig == SIGTTOU )
			{
				return "SIGTTOU";
			}

			if ( _sig == SIGURG )
			{
				return "SIGURG";
			}

			if ( _sig == SIGXCPU )
			{
				return "SIGXCPU";
			}

			if ( _sig == SIGXFSZ )
			{
				return "SIGXFSZ";
			}

			if ( _sig == SIGVTALRM )
			{
				return "SIGVTALRM";
			}

			if ( _sig == SIGPROF )
			{
				return "SIGPROF";
			}

			if ( _sig == SIGWINCH )
			{
				return "SIGWINCH";
			}

#ifndef __FreeBSD__

			if ( _sig == SIGPOLL )
			{
				return "SIGPOLL";
			}

#endif

			if ( _sig == SIGIO )
			{
				return "SIGIO";
			}

#ifndef __FreeBSD__

			if ( _sig == SIGPWR )
			{
				return "SIGPWR";
			}

#endif

			if ( _sig == SIGSYS )
			{
				return "SIGSYS";
			}

			return string( "UNKNOWN(" ) + num_to_str( _sig ) + string( ")" );
		}

		void configure_logging( int _fd, const LOGGING::ENUM_LOG_LEVEL& _level )
		{
			root_log_configurator = new LOGGING::LOG_CONFIGURATOR( _fd, _level );
		}

		extern void destroy_watchdog()
		{
			if ( watchdog != nullptr )
			{
				watchdog->stop_thread();
			}

			return;
		}

		void configure_watchdog( void )
		{
			if ( watchdog == nullptr )
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
			if ( in_trap == true )
			{
				return;
			}

			LOG_INFO( string( "Received signal: " ) + sig_to_str( sig ) + " (" + num_to_str( sig ) + ")" );
			LOG_DEBUG( string( "********************************************************************" ) );
			in_trap = true;
			THREAD_REGISTRY::stop_all();
			GLOBALS::global_exit_flag = true;
			LOG_DEBUG( string( "Cleanup finished.  Exiting." ) );
			return;
		}

		void configure_signals( void )
		{
			struct sigaction sa;
			memset( &sa, 0, sizeof( struct sigaction ) );
			sa.sa_handler = signal_trap;

			if ( sigaction( SIGINT, &sa, nullptr ) != 0 ) //2
			{
				LOG_ERROR( create_perror_string( "Failed to install signal handler for SIGINT" ) );
			}

			/*
			 if (sigaction(SIGKILL, &sa, nullptr) != 0)
			 {
			 LOG_ERROR_STAT(create_perror_string("Failed to install signal handler for SIGKILL"));
			 }*/

			if ( signal( SIGPIPE, SIG_IGN ) != 0 )
			{
				LOG_ERROR( create_perror_string( "Failed to ignore SIGPIPE" ) );
			}

			if ( sigaction( SIGABRT, &sa, nullptr ) != 0 ) //6
			{
				LOG_ERROR( create_perror_string( "Failed to install signal handler for SIGABRT" ) );
			}

			if ( sigaction( SIGFPE, &sa, nullptr ) != 0 ) //8
			{
				LOG_ERROR( create_perror_string( "Failed to install signal handler for SIGFPE" ) );
			}

			if ( sigaction( SIGILL, &sa, nullptr ) != 0 ) //4
			{
				LOG_ERROR( create_perror_string( "Failed to install signal handler for SIGILL" ) );
			}

			if ( sigaction( SIGSEGV, &sa, nullptr ) != 0 )
			{
				LOG_ERROR( create_perror_string( "Failed to install signal handler for SIGSEGV" ) );
			}

			if ( sigaction( SIGTERM, &sa, nullptr ) != 0 )
			{
				LOG_ERROR( create_perror_string( "Failed to install signal handler for SIGTERM" ) );
			}

			if ( sigaction( SIGHUP, &sa, nullptr ) != 0 )
			{
				LOG_ERROR( create_perror_string( "Failed to install signal handler  for SIGHUP" ) );
			}

			return;
		}

		void daemon_self( const char* _pid_file_name )
		{

			int i = fork();

			if ( i < 0 )
			{
				LOG_ERROR( create_perror_string( "Failed to fork" ) );
				exit( -1 );
			}

			if ( i > 0 )
			{
				exit( 0 ); /* parent exits */
			}

			/* child (daemon) continues */

			setsid(); /* obtain a new process group */

			umask( S_IWGRP | S_IWOTH );

			for ( i = getdtablesize(); i >= 0; --i )
			{
				close( i ); /* close all descriptors */
			}

			i = open( "/dev/null", O_RDWR ); /* open stdin */
			dup( i ); /* stdout */
			dup( i ); /* stderr */

			/*
			We're pretty much ignoring all error handling here.
			*/
			std::string pid_txt = num_to_str( getpid() ) + "\n";
			i = open( _pid_file_name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR |  S_IRGRP | S_IROTH );
			write( i, pid_txt.data(), pid_txt.size() );
			close( i );



			return;
		}

		void exit_function( void )
		{
			if ( unlink( GC_PID_FILE ) != 0 )
			{
				LOG_ERROR( create_perror_string( "Failed to unlink PID file" ) );
			}

			return;
		}

		/**
		 * Checks to see if privileges need to be dropped.
		 * \return 1 if the process has a GID or UID of 0
		 */
		int check_privs( void )
		{
			uid_t uid = getuid();
			gid_t gid = getgid();

			if ( uid == 0 || gid == 0 )
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}

		/**
		 * Drops the privileges of the process.  The user and group IDs used are defined in lib/config.hpp
		 */
		void drop_privs( void )
		{
			if ( check_privs() == 0 )
			{
				return;
			}

			int rc = 0;
			struct passwd* pwd = 0;
			struct group* grp = 0;
			errno = 0;
			pwd = getpwnam( GC_PROC_USER );

			if ( pwd == 0 )
			{
				perror( "Failed to get process user pwd entry.  Aborting." );
				exit( -1 );
			}

			errno = 0;
			grp = getgrnam( GC_PROC_GROUP );

			if ( grp == 0 )
			{
				perror( "Failed to get process group grp entry.  Aborting." );
				exit( -2 );
			}

			uid_t uid = pwd->pw_uid;
			gid_t gid = grp->gr_gid;

			if ( getgid() != gid )
			{
				errno = 0;
				rc = setgid( gid );

				if ( rc != 0 )
				{
					perror( "Failed to drop group privileges.  Aborting." );
					exit( -4 );
				}
			}

			if ( getuid() != uid )
			{
				errno = 0;
				rc = setuid( uid );

				if ( rc != 0 )
				{
					perror( "Failed to drop user privileges.  Aborting." );
					exit( -4 );
				}
			}

			return;
		}


		int create_logger_fd( const COMMAND_LINE_PARMS& _clp, bool _test )
		{
			string log_file_name = _clp.get_log_file();

			if ( log_file_name == "-" )
			{
				return 1;
			}

			if ( _test )
			{
				std::cout << "Logging in: [" << log_file_name << "]" << std::endl;
			}

			int fd = open( log_file_name.data(), O_WRONLY | O_APPEND | O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP );

			if ( fd < 0 )
			{
				std::cerr << create_perror_string( "Failed to open log file" )  << std::endl;
				return ( -1 );
			}

			if ( _test )
			{

				if ( write( fd, "\n\n", 2 ) < 0 )
				{
					std::cerr << create_perror_string( "Failed to write to log file" ) << std::endl;
					return ( -1 );
				}
			}

			if ( _test )
			{
				close( fd );
				return 0;
			}
			else
			{
				return fd;
			}
		}

		unsigned long long int get_time_usec( void )
		{
			struct timeval tv;

			gettimeofday( &tv, nullptr );

			return ( ( unsigned long long int )tv.tv_sec * ( unsigned long long int )1000000 + ( unsigned long long int )tv.tv_usec );
		}

	} // END namespace GLOBALS
} // END namespace BBB_HVAC
