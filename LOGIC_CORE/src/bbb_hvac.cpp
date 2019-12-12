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


#include "lib/bbb_hvac.hpp"
#include "lib/message_processor.hpp"
#include "lib/socket_reader.hpp"
#include "lib/logger.hpp"
#include "lib/exceptions.hpp"
#include "lib/string_lib.hpp"


#include "lib/config.hpp"
#include "lib/message_types.hpp"
#include "lib/threads/logic_thread.hpp"
#include "lib/threads/HVAC_logic_loop.hpp"
#include "lib/threads/serial_io_thread.hpp"
#include "lib/threads/shim_listener_thread.hpp"
#include "lib/threads/serial_io_thread.hpp"
#include "lib/threads/thread_registry.hpp"

#include "lib/globals.hpp"
#include "lib/configurator.hpp"


#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <errno.h>

#include <pwd.h>
#include <grp.h>

#include <signal.h>
#include <time.h>
#include <pthread.h>

#include <iostream>
#include <memory>

using namespace BBB_HVAC;
using namespace BBB_HVAC::SERVER;

DEF_LOGGER_STAT( "BBB_HVAC(MAIN)" );

static CONFIGURATOR* config = nullptr;

class COMMAND_LINE_PARMS
{
	public:
		COMMAND_LINE_PARMS( size_t _argc, const char** _argv ) {
			this->argc = _argc;
			this->argv = _argv;

			this->verbose_flag = false;
			this->server_mode = false;

			address = "";
			port_s = "";

			port_i =  GC_DEFAULT_TCPIP_PORT;

			this->st = SOCKET_TYPE::NONE;

			this->exe = string( argv[0] );

			return;
		}

		void process( void ) {
			for ( size_t i = 1; i < argc; i++ ) {
				string p = trimmed( argv[i] );

				if ( p == "-d" ) {
					if ( st != SOCKET_TYPE::NONE ) {
						print_cmd_error( exe, "Confusing command line.  Connection method specified more than once?" );
					}

					st = SOCKET_TYPE::DOMAIN;
				}
				else if ( p == "-i" ) {
					if ( st != SOCKET_TYPE::NONE ) {
						print_cmd_error( exe, "Confusing command line.  Connection method specified more than once?" );
					}

					st = SOCKET_TYPE::TCPIP;
				}
				else if ( p == "-h" ) {
					print_help( exe );
					exit( 0 );
				}
				else if ( p == "-v" ) {
					verbose_flag = true;
				}
				else if ( p == "-p" ) {
					if ( i == ( argc - 1 ) ) {
						print_cmd_error( exe, "No port specified for the -p parameter." );
					}
					else {
						port_s = string( argv[i + 1] );
						i += 1;
					}
				}
				else if ( p == "-a" ) {
					if ( i == ( argc - 1 ) ) {
						print_cmd_error( exe, "No address specified for the -a parameter." );
					}
					else {
						address = string( argv[i + 1] );
						i += 1;
					}
				}
				else if ( p == "-s" ) {
					server_mode = true;
				}
				else {
					print_cmd_error( exe, "Unrecognized command line parameter: " + p );
				}
			}

			if ( st == SOCKET_TYPE::NONE ) {
				print_cmd_error( exe, "Need to specify connection method.  See {-d|-i} parameters." );
			}

			switch ( st ) {
				case SOCKET_TYPE::NONE:
					// do nothing.
					break;

				case SOCKET_TYPE::DOMAIN:
					if ( address.length() == 0 ) {
						address = GC_LOCAL_COMMAND_SOCKET;
					}

					break;

				case SOCKET_TYPE::TCPIP:
					if ( address.length() == 0 ) {
						address = GC_DEFAULT_LISTEN_INTERFACE;
					}

					break;
			}

			if ( verbose_flag ) {
				cout << "Verbose flag (-v) has been set via command line." << endl;
				cout << "    Connection type {-d|-i}: ";

				switch ( st ) {
					case SOCKET_TYPE::NONE:
						cout << "NONE";
						break;

					case SOCKET_TYPE::DOMAIN:
						cout << "DOMAIN";
						break;

					case SOCKET_TYPE::TCPIP:
						cout << "TCPIP";
						break;
				}

				cout << endl;

				cout << "    Address [-a]: [" << address << "]" << endl;
				cout << "    Port [-p]: [" << port_i << "]" << endl;
				cout << "    Server mode [-s]: [" << ( server_mode ? "TRUE" : "FALSE" ) << "]" << endl;
			}

		}

		bool is_verbose_flag( void ) const {
			return this->verbose_flag;
		}

		bool is_server_mode( void ) const {
			return this->server_mode;
		}

		const string& get_address( void ) const {
			return this->address;
		}

		uint16_t get_port( void ) const {
			return this->port_i;
		}

		SOCKET_TYPE get_socket_type( void ) const {
			return this->st;
		}

		const string& get_exe( void ) const {
			return this->exe;
		}
	protected:

		void print_help( const string& _c ) {
			cout << "Usage: " << endl;
			cout << _c << " {-d|i} [-a <address>] [-p <port>] [-s] [-v]" << endl;
			cout << "Where: " << endl;
			cout << "\t-d - Listen on domain socket (mutually exclusive with -i)" << endl;
			cout << "\t-i - Listen on TCP/IP socket (mutually exclusive with -d)" << endl;
			cout << "\t-a <address> - Address to listen on.  File name (optional) if -d is specified.  Interface to bind to if -i is specified." << endl;
			cout << "\t  For -i, the interface should be specified in x.x.x.x notation. Default is 127.0.0.1." << endl;
			cout << "\t-p <port> - Port to listen to.  Relevant only if -i is specified.  Defaults to 6666" << endl;
			cout << "\t-s - Server mode.  If not specified application runs in console." << endl;
			cout << "\t-v - Verbose mode.  Produces extra debugging information to the console." << endl;
			cout << "\t-h - This help" << endl;
			exit( -1 );

			return;
		}

		void print_cmd_error( const string& _c, const string& _failure ) {
			cerr << "Command line error: " << endl;
			cerr << _failure << endl;
			print_help( _c );
		}


		size_t argc;
		const char** argv;
	private:
		bool verbose_flag;
		bool server_mode;
		string address;
		string port_s;
		uint16_t port_i;
		SOCKET_TYPE st;
		string exe;


};

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
bool start_board_io_thread( const CONFIG_ENTRY& _board_config )
{
	const string board_name = _board_config.get_part_as_string( 0 );
	const string board_dev = _board_config.get_part_as_string( 1 );
	bool debug = false;

	if ( _board_config.get_part_count() == 3 )
	{
		if ( _board_config.get_part_as_string( 2 ) == "DEBUG" )
		{
			LOG_INFO_STAT( "Starting board " + board_name + " in debug mode." );
			debug = true;
		}
	}

	LOG_DEBUG_STAT( "Starting thread for board: " + board_name );
	IOCOMM::SER_IO_COMM* ser_comm	= new IOCOMM::SER_IO_COMM( board_dev.data(), board_name, debug );

	if ( ser_comm->init() != IOCOMM::ENUM_ERRORS::ERR_NONE )
	{
		LOG_ERROR_STAT( "Failed to initialized serial IO for board:" + board_name );
		return false;
	}

	ser_comm->start_thread();
	return true;
}
bool start_io_threads( CONFIGURATOR* config )
{
	const CONFIG_TYPE_INDEX_TYPE& board_config = config->get_board_index();

	for ( CONFIG_TYPE_INDEX_TYPE::const_iterator i = board_config.begin(); i != board_config.end(); ++i )
	{
		const CONFIG_ENTRY& bc = config->get_config_entry( *i );

		if ( !start_board_io_thread( bc ) )
		{
			return false;
		}
	}

	return true;
}

bool start_shim_thread( const COMMAND_LINE_PARMS& _clp )
{
	SHIM_LISTENER* listener = new SHIM_LISTENER( _clp.get_socket_type(), _clp.get_address(), _clp.get_port() );

	try
	{
		listener->init();
	}
	catch ( const exception& _e )
	{
		LOG_ERROR_STAT( "Failed to initialize shim listener thread: " + string( _e.what() ) );
		delete listener;
		return false;
	}

	listener->start_thread();
	return true;
}

bool start_logic_thread( CONFIGURATOR* )
{
	GLOBALS::logic_instance = new HVAC_LOGIC::HVAC_LOGIC_LOOP( config );
	/*
	 * HVAC_LOGIC_LOOP takes ownership of the CONFIGURATOR instance.
	 */
	GLOBALS::logic_instance->start_thread();
	return true;
}

bool start_threads( CONFIGURATOR* config, const COMMAND_LINE_PARMS& _clp )
{
	GLOBALS::configure_watchdog();

	if ( !start_shim_thread( _clp ) )
	{
		LOG_ERROR_STAT( "Failed to start shim listener thread." );
		return false;
	}

	if ( !start_io_threads( config ) )
	{
		LOG_ERROR_STAT( "Failed to start IO threads." );
		return false;
	}

	sleep( 2 );

	if ( !start_logic_thread( config ) )
	{
		LOG_ERROR_STAT( "Failed to start logic thread." );
		return false;
	}

	return true;
}

void io_death_listener( const std::string& _tag )
{
	LOG_DEBUG_STAT( "IO thread [" + _tag + "] death sensed." );
	const CONFIG_TYPE_INDEX_TYPE& board_config = config->get_board_index();

	for ( CONFIG_TYPE_INDEX_TYPE::const_iterator i = board_config.begin(); i != board_config.end(); ++i )
	{
		const CONFIG_ENTRY& bc = config->get_config_entry( *i );
		const string board_name = bc.get_part_as_string( 0 );

		if ( board_name == _tag )
		{
			if ( !start_board_io_thread( bc ) )
			{
				LOG_ERROR_STAT( "Failed to re-start IO board thread.  Aborting." );
				GLOBALS::global_exit_flag = true;
			}
		}
	}

	return;
}

int do_main( const COMMAND_LINE_PARMS& _clp )
{
	GLOBALS::configure_logging( LOGGING::ENUM_LOG_LEVEL::DEBUG );
	LOG_INFO_STAT( "Starting up BBB HVAC server." );
	GLOBALS::configure_signals();
	drop_privs();
	config = new CONFIGURATOR( "configuration.cfg" );
	config->read_file();
	//start_io_threads(config);

	if ( !start_threads( config, _clp ) )
	{
		LOG_ERROR_STAT( "Failed to start threads." );
		GLOBALS::global_exit_flag = true;
		THREAD_REGISTRY::stop_all();
	}
	else
	{
		THREAD_REGISTRY::register_io_death_listener( io_death_listener );

		while ( GLOBALS::global_exit_flag == false )
		{
			sleep( 1 );
			THREAD_REGISTRY::global_cleanup();
		}
	}

	THREAD_REGISTRY::stop_all();
	THREAD_REGISTRY::destroy_global();
	LOG_INFO_STAT( "Main process is exiting." );
	LOGGING::LOG_CONFIGURATOR::destroy_root_configurator();
	return EXIT_SUCCESS;
}




int main( int argc, const char** argv )
{
	COMMAND_LINE_PARMS clp( argc, argv );
	clp.process();
	do_main( clp );
}
