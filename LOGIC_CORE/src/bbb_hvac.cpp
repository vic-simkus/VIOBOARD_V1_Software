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
#include "lib/log_configurator.hpp"
#include "lib/globals.hpp"
#include "lib/configurator.hpp"

#include "command_line_parms.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <errno.h>

#include <pwd.h>
#include <grp.h>

#include <signal.h>
#include <time.h>
#include <pthread.h>

#include <memory>
#include <iostream>

using namespace BBB_HVAC;
using namespace BBB_HVAC::SERVER;

DEF_LOGGER_STAT( "BBB_HVAC(MAIN)" );

static CONFIGURATOR* config = nullptr;

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
			LOG_INFO( "Starting board " + board_name + " in debug mode." );
			debug = true;
		}
	}

	LOG_DEBUG( "Starting thread for board: " + board_name );
	IOCOMM::SER_IO_COMM* ser_comm	= new IOCOMM::SER_IO_COMM( board_dev.data(), board_name, debug );

	if ( ser_comm->init() != IOCOMM::ENUM_ERRORS::ERR_NONE )
	{
		LOG_ERROR( "Failed to initialized serial IO for board:" + board_name );
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

/**
Starts the shim listener thread.  This is where we bind to either the domain socket or network interface.
*/
bool start_shim_thread( const COMMAND_LINE_PARMS& _clp )
{
	SHIM_LISTENER* listener = new SHIM_LISTENER( _clp.get_socket_type(), _clp.get_address(), _clp.get_port() );

	try
	{
		listener->init();
	}
	catch ( const exception& _e )
	{
		LOG_ERROR( "Failed to initialize shim listener thread: " + string( _e.what() ) );
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

void io_death_listener( const std::string& _tag )
{
	LOG_DEBUG( "IO thread [" + _tag + "] death sensed." );
	const CONFIG_TYPE_INDEX_TYPE& board_config = config->get_board_index();

	for ( CONFIG_TYPE_INDEX_TYPE::const_iterator i = board_config.begin(); i != board_config.end(); ++i )
	{
		const CONFIG_ENTRY& bc = config->get_config_entry( *i );
		const string board_name = bc.get_part_as_string( 0 );

		if ( board_name == _tag )
		{
			if ( !start_board_io_thread( bc ) )
			{
				LOG_ERROR( "Failed to re-start IO board thread.  Aborting." );
				GLOBALS::global_exit_flag = true;
			}
		}
	}

	return;
}

bool start_threads( CONFIGURATOR* config, const COMMAND_LINE_PARMS& _clp )
{
	GLOBALS::configure_watchdog();

	if ( !start_shim_thread( _clp ) )
	{
		LOG_ERROR( "Failed to start shim listener thread." );
		return false;
	}

	if ( !start_io_threads( config ) )
	{
		LOG_ERROR( "Failed to start IO threads." );
		return false;
	}

	sleep( 2 );

	if ( !start_logic_thread( config ) )
	{
		LOG_ERROR( "Failed to start logic thread." );
		return false;
	}

	return true;
}

int do_main( const COMMAND_LINE_PARMS& _clp )
{

	string log_file_name = _clp.get_log_file();

	std::cout << "Logging in: [" << log_file_name << "]" << std::endl;

	int fd = open( log_file_name.data(), O_WRONLY | O_APPEND | O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP );

	if ( fd < 0 )
	{
		std::cerr << create_perror_string( "Failed to open log file" )  << std::endl;
		exit( -1 );
	}

	if ( write( fd, "TEST WRITE\n", 11 ) < 0 )
	{
		std::cerr << create_perror_string( "Failed to write to log file" ) << std::endl;
		exit( -1 );
	}

	GLOBALS::configure_logging( fd, LOGGING::ENUM_LOG_LEVEL::TRACE );

	LOG_INFO( "Starting up BBB HVAC server." );

	GLOBALS::configure_signals();

	drop_privs();

	config = new CONFIGURATOR( "configuration.cfg" );
	config->read_file();

	//start_io_threads(config);

	if ( !start_threads( config, _clp ) )
	{
		LOG_ERROR( "Failed to start threads." );
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

	LOG_INFO( "Main process is exiting." );

	LOGGING::LOG_CONFIGURATOR::destroy_root_configurator();

	return EXIT_SUCCESS;
}

int main( int argc, const char** argv )
{
	COMMAND_LINE_PARMS clp( argc, argv );

	// If there is an error in command line parms this method never returns.
	clp.process();

	// No problem with the command line parameters, do our main thing.
	return do_main( clp );
}
