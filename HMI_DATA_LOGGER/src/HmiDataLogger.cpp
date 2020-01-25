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

#include "lib/logger.hpp"

#include "lib/threads/thread_registry.hpp"

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
		LOG_DEBUG( "log rotate size: " + num_to_str( _config.rotate_size ) );
		LOG_DEBUG( "log dir: " + _config.log_dir );
		LOG_DEBUG( "base data file name: " + _config.base_data_file_name );
		LOG_DEBUG( "current file index: " + num_to_str( _config.current_file_index ) );
		LOG_DEBUG( "fail hard: " + num_to_str( _config.fail_hard ) );
		LOG_DEBUG( "mode: " + Config::mode_to_string( _config.mode ) );
		LOG_DEBUG( "pg url: " + _config.pg_url );
		return;
	}
}

bool collect_data_loop( std::shared_ptr<HMI_DATA_LOGGER::Connection> _connection )
{
	if ( !_connection->connect() )
	{
		return false;
	}

	while ( !BBB_HVAC::GLOBALS::global_exit_flag )
	{
		if ( !_connection->read_status() )
		{
			LOG_ERROR( "Failed to read status.  See previous error logs." );
			_connection->disconnect();
			return false;
		}

		sleep( 1 );
	}

	return true;
}
bool dump_fields( std::shared_ptr<HMI_DATA_LOGGER::Connection> _connection )
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

	return true;
}
bool collect_data( HMI_DATA_LOGGER::Context* _logger_context )
{
	switch ( _logger_context->configuration.mode )
	{
		case HMI_DATA_LOGGER::Config::MODE::NONE:
			throw logic_error( "Mode is none." );

		case HMI_DATA_LOGGER::Config::MODE::FILE:
			return collect_data_loop( std::shared_ptr<HMI_DATA_LOGGER::Connection>( new HMI_DATA_LOGGER::ConnectionFile( _logger_context ) ) );
			break;

		case HMI_DATA_LOGGER::Config::MODE::PGSQL:
			return collect_data_loop( std::shared_ptr<HMI_DATA_LOGGER::Connection>( new HMI_DATA_LOGGER::ConnectionPgsql( _logger_context ) ) );
			break;

		case HMI_DATA_LOGGER::Config::MODE::PFIELDS:
			return dump_fields( std::shared_ptr<HMI_DATA_LOGGER::Connection>( new HMI_DATA_LOGGER::ConnectionFile( _logger_context ) ) );
			break;

		default:
			throw logic_error( "Unrecognized mode: " + HMI_DATA_LOGGER::Config::mode_to_string( _logger_context->configuration.mode ) );
	}
}
int main( int argc, const char** argv )
{
	BBB_HVAC::GLOBALS::configure_logging( 1, BBB_HVAC::LOGGING::ENUM_LOG_LEVEL::ERROR );
	BBB_HVAC::GLOBALS::configure_signals();


	LOG_INFO( "Starting." );

	HMI_DATA_LOGGER::Context logger_context = HMI_DATA_LOGGER::create_context( argc, argv );

	dump_config( logger_context.configuration );

	if ( logger_context.check_data_dir() == false )
	{
		LOG_ERROR( "Check of data log directory failed.  See previous error messages for hints.  Bailing." );
		return -1;
	}

	LOG_DEBUG( "Next data file: " + logger_context.get_next_data_file_name() );

	try
	{
		/*
		We can retry indefinitely on recoverable errors.  On Exceptions we bail.
		*/

		if ( logger_context.configuration.fail_hard == true )
		{
			collect_data( &logger_context );
			LOG_DEBUG( "collect_data returned." );
		}
		else
		{
			LOG_INFO( "Application is running in FAIL_HARD=FALSE mode.  Will retry indefinitely on all error conditions." );

			while ( 1 )
			{
				if ( collect_data( &logger_context ) )
				{
					LOG_INFO( "Data collection ended successfully.  Exiting." );
					break;
				}
				else
				{
					LOG_ERROR( "Error in data collection process.  FAIL_HARD is false.  Looping." );
				}

				if ( BBB_HVAC::GLOBALS::global_exit_flag == true )
				{
					LOG_INFO( "Exiting because BBB_HVAC::GLOBALS::global_exit_flag is true." );
					break;
				}

				BBB_HVAC::THREAD_REGISTRY::global_cleanup();
				sleep( 1 );
			}
		}
	}
	catch ( const exception& _e )
	{
		LOG_ERROR( "Unexpected exception caught: " + string( _e.what() ) );
	}

	LOG_INFO( "Application is exiting." );

	return 0;
}