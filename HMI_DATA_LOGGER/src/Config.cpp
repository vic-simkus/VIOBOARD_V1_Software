
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

#include "include/Config.hpp"
#include "include/Context.hpp"

#include "lib/logger.hpp"

using namespace HMI_DATA_LOGGER;

DEF_LOGGER_STAT( "HMI_DATA_LOGGER::CONFIG" );

bool HMI_DATA_LOGGER::create_configuration( Context& _ctx, int _argc, const char** _argv )
{
	bool rc = true;

	BBB_HVAC::COMMAND_LINE_PARMS::EX_PARAM_LIST ex_parms;

	ex_parms[CFG_CMDP_FAIL_HARD] = "If set to true the application will fail 'hard and loud' on error conditions.\n\t\tIf [false] (default) the application will keep trying failed operations.";
	ex_parms[CFG_CMDP_ROTATE_SIZE] = "Size of the data log file, in bytes, when it should be rotated.\n\t\tRelevant only if mode is FILE.";
	ex_parms[CFG_CMDP_LOG_DIR] = "Directory where the logged data will be saved to.\n\t\tRelevant only if mode is FILE.";
	ex_parms[CFG_CMDP_BASE_DATA_FILE] = "Base name of the data file.\n\t\tRelevant only if mode is FILE.";
	ex_parms[CFG_CMDP_MODE] = "Where to save the data [FILE|PGSQL|PFIELDS]\n\t\tSpecify --pg_host if mode = PGSQL";
	ex_parms[CFG_CMDP_PG_URL] = "PostgreSQL connection string.\n\t\tRelevant only if mode is PGSQL.\n\t\tExample URL: postgresql://[user[:password]@][netloc][:port][/dbname][?param1=value1&...]";
	ex_parms[CFG_CMDP_PG_TEST] = "Try to connect to PostgreSQL server.\n\t\tMust supply connection URL via --pg_url and specify --mode as PGSQL.";

	LOG_DEBUG( "Processing command line parameters." );

	std::unique_ptr<char> _real_path_ptr( realpath( _argv[0], nullptr ) );

	if ( !_real_path_ptr )
	{
		LOG_ERROR( BBB_HVAC::create_perror_string( "Failed to normalize our own executable path?" ) );
		return false;
	}

	std::string normalized_me = std::string( _real_path_ptr.get() );
	_ctx.set_prog_name( std::string( _argv[0] ) );
	_ctx.set_prog_name_fixed( normalized_me );

	_ctx.configuration.set_command_line_parms( new BBB_HVAC::COMMAND_LINE_PARMS( ( size_t )_argc, _argv, ex_parms ) );

	const BBB_HVAC::COMMAND_LINE_PARMS::EX_PARAM_LIST& parm_list = _ctx.configuration.get_command_line_parms();

	for ( auto i = parm_list.begin(); i != parm_list.end(); ++i )
	{
		std::string param = i->first;

		LOG_DEBUG( "Param [" + i->first + "]: " + i->second );

		if ( param == CFG_CMDP_ROTATE_SIZE )
		{
			size_t rotate_size;

			try
			{
				rotate_size = std::stoul( i->second );
			}
			catch ( const std::exception& e )
			{
				LOG_ERROR( "Failed to convert value o LOG_SIZE parameter to number.  Value [" + i->second + "], error: " + e.what() );
				return false;
			}

			_ctx.configuration.rotate_size = rotate_size;
		}
		else if ( param == CFG_CMDP_LOG_DIR )
		{
			_ctx.configuration.log_dir = i->second;
		}
		else if ( param == CFG_CMDP_BASE_DATA_FILE )
		{
			_ctx.configuration.base_data_file_name =  i->second;
		}
		else if ( param == CFG_CMDP_PG_URL )
		{
			_ctx.configuration.pg_url =  i->second;
		}
		else if ( param == CFG_CMDP_MODE )
		{
			string mode = to_upper_case( i->second );

			if ( mode == "FILE" )
			{
				_ctx.configuration.mode = Config::MODE::FILE;
			}
			else if ( mode == "PGSQL" )
			{
				_ctx.configuration.mode = Config::MODE::PGSQL;
			}
			else if ( mode == "PFIELDS" )
			{
				_ctx.configuration.mode = Config::MODE::PFIELDS;
			}
		}
		else if ( param == CFG_CMDP_FAIL_HARD )
		{
			if ( to_upper_case( i->second ) == "TRUE" )
			{
				_ctx.configuration.fail_hard = true;
			}
			else
			{
				_ctx.configuration.fail_hard = false;
			}
		}
	}

	_ctx.configuration.valid = true;

	return rc;

}

Config::Config()
{
	this->rotate_size = 0;
	this->log_dir = "./log_data";
	this->base_data_file_name = "sys_status_log.#.csv";
	this->current_file_index = 0;
	this->fail_hard = true;
	this->mode = MODE::NONE;
	this->valid = false;

	return;
}

bool Config::does_command_line_exist( const std::string& _name )
{
	if ( command_line_parms->ex_parm_values.count( _name ) == 1 )
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::string Config::get_command_line_value( const std::string& _name )
{
	if ( command_line_parms->ex_parm_values.count( _name ) > 0 )
	{
		return command_line_parms->ex_parm_values[_name];
	}
	else
	{
		return "";
	}
}

void Config::set_command_line_parms( BBB_HVAC::COMMAND_LINE_PARMS* _parms )
{
	this->command_line_parms.reset( _parms );

	// If there is an error in command line parms this method never returns.
	this->command_line_parms->process();

	return;
}

const BBB_HVAC::COMMAND_LINE_PARMS::EX_PARAM_LIST  Config::get_command_line_parms( void ) const
{
	return this->command_line_parms->ex_parm_values;
}
const std::shared_ptr<BBB_HVAC::COMMAND_LINE_PARMS> Config::get_command_line_processor( void ) const
{
	return this->command_line_parms;
}