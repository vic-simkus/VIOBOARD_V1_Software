/*
Copyright (C) 2019  Vidas Simkus (vic.simkus@simkus.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "include/Context.hpp"
#include "include/ConnectionFile.hpp"
#include "include/ConnectionPgsql.hpp"
#include "include/Exception.hpp"

#include "lib/logger.hpp"
#include "lib/threads/thread_registry.hpp"
#include "lib/globals.hpp"

/*
For sleep
*/
#include <unistd.h>

#include <iostream>

DEF_LOGGER_STAT( "HMI_DATA_LOGGER::MAIN" );

/**
Application namespace
*/
namespace HMI_DATA_LOGGER
{
	static void dump_config( const Config& _config )
	{
		LOG_DEBUG( "Application configuration:" );
		LOG_DEBUG( "  log rotate size: " + num_to_str( _config.rotate_size ) );
		LOG_DEBUG( "  log dir: " + _config.log_dir );
		LOG_DEBUG( "  base data file name: " + _config.base_data_file_name );
		LOG_DEBUG( "  current file index: " + num_to_str( _config.current_file_index ) );
		LOG_DEBUG( "  fail hard: " + num_to_str( _config.fail_hard ) );
		LOG_DEBUG( "  mode: " + Config::mode_to_string( _config.mode ) );
		LOG_DEBUG( "  pg url: " + _config.pg_url );
		return;
	}
}

void collect_data_loop( std::shared_ptr<HMI_DATA_LOGGER::Connection> _connection )
{
	try
	{
		_connection->connect();
	}
	catch ( const HMI_DATA_LOGGER::Exception& _e )
	{
		throw HMI_DATA_LOGGER::Exception( __FILE__, __LINE__, __FUNCTION__, "Failed to connect", HMI_DATA_LOGGER::ExceptionPtr( new HMI_DATA_LOGGER::Exception( _e ) ) );
	}

	while ( !BBB_HVAC::GLOBALS::global_exit_flag )
	{
		try
		{
			_connection->read_status();

			if ( _connection->logger_context->fail_flag )
			{
				LOG_INFO( "Clearing failure flag." );
				_connection->logger_context->fail_flag = false;
				_connection->logger_context->fail_count = 0;
			}
		}
		catch ( const HMI_DATA_LOGGER::Exception& _e )
		{
			try
			{
				_connection->disconnect();
			}
			catch ( ... )
			{
				//ignore error at this point.
			}

			throw HMI_DATA_LOGGER::Exception( __FILE__, __LINE__, __FUNCTION__, "Failed to read status.", HMI_DATA_LOGGER::ExceptionPtr( new HMI_DATA_LOGGER::Exception( _e ) ) );
		}

		sleep( 1 );
	}

	LOG_DEBUG( "Data collection finished." );
}
void dump_fields( std::shared_ptr<HMI_DATA_LOGGER::Connection> _connection )
{
	_connection->connect();
	std::list<std::string> names = _connection->get_item_names();

	std::cout << endl;
	std::cout << "Field names: " << endl;

	for ( auto i = names.begin(); i != names.end(); ++i )
	{
		std::cout << "\t" << ( *i ) << endl;
	}

	std::cout << endl;
}
void mode_pgsql( HMI_DATA_LOGGER::Context* _logger_context )
{
	std::shared_ptr<HMI_DATA_LOGGER::ConnectionPgsql> pgsql_conn( new HMI_DATA_LOGGER::ConnectionPgsql( _logger_context ) );

	if ( _logger_context->configuration.does_command_line_exist( CFG_CMDP_PG_TEST ) )
	{
		if ( pgsql_conn->test_connection() )
		{
			LOG_INFO( "PostgreSQL connection test succeeded." );
			return;
		}
		else
		{
			LOG_ERROR( "PostgreSQL connection test failed." );
			return;
		}
	}
	else
	{
		collect_data_loop( pgsql_conn );
	}
}
void  mode_file( HMI_DATA_LOGGER::Context* _logger_context )
{
	collect_data_loop( std::shared_ptr<HMI_DATA_LOGGER::Connection>( new HMI_DATA_LOGGER::ConnectionFile( _logger_context ) ) );
}
void collect_data( HMI_DATA_LOGGER::Context* _logger_context )
{
	switch ( _logger_context->configuration.mode )
	{
		case HMI_DATA_LOGGER::Config::MODE::NONE:
			throw HMI_DATA_LOGGER::Exception( __FILE__, __LINE__, __FUNCTION__, "Mode is none." );

		case HMI_DATA_LOGGER::Config::MODE::FILE:
			mode_file( _logger_context );
			break;

		case HMI_DATA_LOGGER::Config::MODE::PGSQL:
			mode_pgsql( _logger_context );
			break;

		case HMI_DATA_LOGGER::Config::MODE::PFIELDS:
			dump_fields( std::shared_ptr<HMI_DATA_LOGGER::Connection>( new HMI_DATA_LOGGER::ConnectionFile( _logger_context ) ) );
			break;

		default:
			throw HMI_DATA_LOGGER::Exception( __FILE__, __LINE__, __FUNCTION__, "Unrecognized mode: " + HMI_DATA_LOGGER::Config::mode_to_string( _logger_context->configuration.mode ) );
	}
}
#define ERROR_SINK 5
int main( int argc, const char** argv )
{
	HMI_DATA_LOGGER::Context logger_context = HMI_DATA_LOGGER::create_context( argc, argv );

	int fd = BBB_HVAC::GLOBALS::create_logger_fd( *logger_context.configuration.get_command_line_processor().get(), true );

	if ( fd < 0 )
	{
		return -1;
	}

	dump_config( logger_context.configuration );

	if ( logger_context.configuration.get_command_line_processor()->is_server_mode() )
	{
		LOG_INFO( "Daemoning self." )
		BBB_HVAC::GLOBALS::daemon_self( logger_context.configuration.pid_file.data() );
	}

	fd = BBB_HVAC::GLOBALS::create_logger_fd( *logger_context.configuration.get_command_line_processor().get(), false );

	BBB_HVAC::GLOBALS::configure_logging( fd, BBB_HVAC::LOGGING::ENUM_LOG_LEVEL::DEBUG );
	BBB_HVAC::GLOBALS::configure_signals();

	LOG_INFO( "Starting." );

	if ( logger_context.configuration.mode == HMI_DATA_LOGGER::Config::MODE::FILE )
	{

		if ( logger_context.check_data_dir() == false )
		{
			LOG_ERROR( "Check of data log directory failed.  See previous error messages for hints.  Bailing." );
			return -1;
		}

		LOG_DEBUG( "Next data file: " + logger_context.get_next_data_file_name() );
	}

	try
	{
		if ( logger_context.configuration.fail_hard == true )
		{
			try
			{
				collect_data( &logger_context );
			}
			catch ( const HMI_DATA_LOGGER::Exception& _e )
			{
				LOG_ERROR( "Data collection failed:\n" + _e.toString() ) ;
			}
			catch ( const std::exception& _e )
			{
				LOG_ERROR( "Data collection failed: " + std::string( _e.what() ) );
			}
		}
		else
		{
			LOG_INFO( "Application is running in FAIL_HARD=FALSE mode.  Will retry indefinitely on all error conditions." );

			while ( 1 )
			{
				try
				{
					collect_data( &logger_context );
					LOG_INFO( "Data collection ended successfully.  Exiting." );
					break;
				}
				catch ( const HMI_DATA_LOGGER::Exception& _e )
				{
					//LOG_ERROR( "Exception: \n" + std::string( _e.toString() ) );

					if ( logger_context.fail_flag == false )
					{
						// This is a first failure

						LOG_ERROR( "Error in data collection process.  FAIL_HARD is false.  Looping." );
						LOG_ERROR( "Exception:\n " + _e.toString() );

						logger_context.fail_flag = true;
						logger_context.fail_count = 0;
					}
					else
					{
						logger_context.fail_count += 1;

						if ( logger_context.fail_count >= ERROR_SINK && ( ( logger_context.fail_count % ERROR_SINK ) == 0 ) )
						{
							LOG_ERROR( "Suppressed previous " + num_to_str( ERROR_SINK ) + " failure messages.  Most recent error:\n" + _e.toString() );
						}
					}

					if ( BBB_HVAC::GLOBALS::global_exit_flag == true )
					{
						LOG_INFO( "Exiting because BBB_HVAC::GLOBALS::global_exit_flag is true." );
						break;
					}

					BBB_HVAC::THREAD_REGISTRY::global_cleanup();
				}//main try/catch block

				sleep( 1 );
			} // retry loop
		}
	}
	catch ( const HMI_DATA_LOGGER::Exception& _e )
	{
		LOG_ERROR( "Unexpected exception caught:\n" + _e.toString() ) ;
	}
	catch ( const std::exception& _e )
	{
		LOG_ERROR( "Unexpected exception caught: " + std::string( _e.what() ) );
	}

	LOG_INFO( "Application is exiting." );

	BBB_HVAC::GLOBALS::global_exit_flag = true;
	sleep( 1 );


	return 0;
}