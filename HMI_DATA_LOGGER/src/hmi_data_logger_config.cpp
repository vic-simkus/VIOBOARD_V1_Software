
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

#include "include/hmi_data_logger_config.hpp"
#include "include/hmi_data_logger_context.hpp"

#include "lib/logger.hpp"

using namespace HMI_DATA_LOGGER;

DEF_LOGGER_STAT( "HMI_DATA_LOGGER::CONFIG" );

bool HMI_DATA_LOGGER::create_configuration( HMI_DATA_LOGGER_CONTEXT& _ctx, int _argc, const char** _argv )
{
	bool rc = true;

	BBB_HVAC::COMMAND_LINE_PARMS::EX_PARAM_LIST ex_parms;

	ex_parms["--fail_hard"] = "If set to true the application will fail 'hard and loud' on error conditions.\n\t\tIf [false] (default) the application will keep trying failed operations.";
	ex_parms["--rotate_size"] = "Size of the data log file, in bytes, when it should be rotated.";
	ex_parms["--log_dir"] = "Directory where the logged data will be saved to.";
	ex_parms["--base_data_file_name"] = "Base name of the data file.";

	_ctx.configuration.command_line_parms.reset( new BBB_HVAC::COMMAND_LINE_PARMS( _argc, _argv, ex_parms ) );

	// If there is an error in command line parms this method never returns.
	_ctx.configuration.command_line_parms->process();

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

	if ( ( _argc % 2 ) == 0 )
	{
		/*
		We always expect an odd number of parameters.
		0   -- program name
		n   -- param name
		n+1 -- param value

		... so forth ...
		*/
		LOG_ERROR( "Command line parameter goofiness.  We always expect an odd number of parameters (including argv[0]!)." );
		return false;
	}

	for ( auto i = _ctx.configuration.command_line_parms->ex_parm_values.begin(); i != _ctx.configuration.command_line_parms->ex_parm_values.begin(); ++i )
	{
		//LOG_DEBUG_STAT("Param [" + num_to_str(i) + "]: " + std::string(_argv[i]) );
		std::string param = i->first;

		if ( param == "--rotate_size" )
		{
			size_t rotate_size;

			try
			{
				rotate_size = std::stoul( i->second );
			}
			catch ( const std::exception& e )
			{
				LOG_ERROR( "Failed to convert value o LOG_SIZE parameter to number.  Value [" + i->second + "], error: " + e.what() );
			}

			_ctx.configuration.rotate_size = rotate_size;
		}
		else if ( param == "--log_dir" )
		{
			_ctx.configuration.log_dir = i->second;
		}
		else if ( param == "--base_data_file_name" )
		{
			_ctx.configuration.base_data_file_name =  i->second;
		}
		else if ( param == "--fail_hard" )
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

	return rc;

}