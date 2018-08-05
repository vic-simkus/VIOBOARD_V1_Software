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

#include "include/hmi_data_logger_context.hpp"
#include "include/hmi_data_logger_connection.hpp"

#include "lib/logger.hpp"


/*
For sleep
*/
#include <unistd.h>

DEF_LOGGER_STAT("HMI_DATA_LOGGER::MAIN");

/**
Application namespace
*/
namespace HMI_DATA_LOGGER
{
	static void dump_config(const HMI_DATA_LOGGER_CONFIG& _config)
	{
		LOG_DEBUG_STAT("Application configuration:");
		LOG_DEBUG_STAT("log rotate size: " + num_to_str(_config.rotate_size));
		LOG_DEBUG_STAT("log dir: " + _config.log_dir);
		LOG_DEBUG_STAT("base data file name: " + _config.base_data_file_name);
		LOG_DEBUG_STAT("current file index: " + num_to_str(_config.current_file_index));
		LOG_DEBUG_STAT("fail hard: " + num_to_str(_config.fail_hard));
		return;
	}

/*
	static bool check_data_need_rotate(const HMI_DATA_LOGGER_CONTEXT& _ctx)
	{

	}
	*/
}

bool process_command_line(int _argc,char ** _argv,HMI_DATA_LOGGER::HMI_DATA_LOGGER_CONTEXT& _ctx)
{
	LOG_DEBUG_STAT("Processing command line parameters.");

	bool rc = true;

	std::unique_ptr<char> _real_path_ptr(realpath(_argv[0],nullptr));

	if(!_real_path_ptr)
	{
		LOG_ERROR_STAT(BBB_HVAC::create_perror_string("Failed to normalize our own executable path?"));
		return false;
	}

	std::string normalized_me = std::string(_real_path_ptr.get());

	_ctx.set_prog_name(std::string(_argv[0]));
	_ctx.set_prog_name_fixed(normalized_me);

	if( (_argc % 2) == 0)
	{
		/*
		We always expect an odd number of parameters.
		0   -- program name
		n   -- param name
		n+1 -- param value

		... so forth ...
		*/

		LOG_ERROR_STAT("Command line parameter goofiness.  We always expect an odd number of parameters (including argv[0]!).");
		return false;
	}

	for(int i=1;i<_argc;i++)
	{
		//LOG_DEBUG_STAT("Param [" + num_to_str(i) + "]: " + std::string(_argv[i]) );

		std::string param = _argv[i];

		if(param == "ROTATE_SIZE")
		{
			size_t rotate_size;
			try
			{
				rotate_size = std::stoul(std::string(_argv[i+1]));
			}
			catch( const std::exception& e)
			{
				LOG_ERROR_STAT("Failed to convert value to LOG_SIZE parameter to number.  Value [" + std::string(_argv[i+1]) + "], error: " + e.what());
				rc = false;
			}

			_ctx.configuration.rotate_size = rotate_size;
			i += 1;
		}
		else if(param == "LOG_DIR")
		{
			_ctx.configuration.log_dir = std::string(_argv[i+1]);
			i += 1;
		}
		else if(param == "BASE_DATA_FILE_NAME")
		{
			_ctx.configuration.base_data_file_name = std::string(_argv[i+1]);
			i += 1;
		}
		else if(param == "FAIL_HARD")
		{
			if(to_upper_case(std::string(_argv[i+1])) == "TRUE")
			{
				_ctx.configuration.fail_hard = true;
			}
			else
			{
				_ctx.configuration.fail_hard = false;
			}
			i += 1;
		}
		else
		{
			LOG_ERROR_STAT("Unrecognized command line parameter [" + param + "]");
			rc = false;
		}

	}

	return rc;
}

bool collect_data(HMI_DATA_LOGGER::HMI_DATA_LOGGER_CONTEXT&)
{
	HMI_DATA_LOGGER::HMI_DATA_LOGGER_CONNECTION connection;

	if(!connection.connect())
	{
		return false;
	}

	return true;
}
int main(int argc,char ** argv)
{
	BBB_HVAC::GLOBALS::configure_logging(BBB_HVAC::LOGGING::ENUM_LOG_LEVEL::DEBUG);
	BBB_HVAC::GLOBALS::configure_signals();

	HMI_DATA_LOGGER::HMI_DATA_LOGGER_CONTEXT logger_context;

	LOG_ERROR_STAT("Starting.");

	if(!process_command_line(argc,argv,logger_context))
	{
		LOG_ERROR_STAT("Command line parameter processing failed.  Please refer to previous error messages.  Program will abort.");
		exit(-1);
	}

	dump_config(logger_context.configuration);

	if(logger_context.check_data_dir() == false)
	{
		LOG_ERROR_STAT("Check of data log directory failed.  See previous error messages for hints.  Bailing.");
		return -1;
	}

	if(logger_context.configuration.fail_hard == true)
	{
		collect_data(logger_context);
	}
	else
	{
		LOG_INFO_STAT("Application is running in FAIL_HARD=FALSE mode.  Will retry indefinitely on all error conditions.");
		while(1)
		{
			if(collect_data(logger_context))
			{
				LOG_DEBUG_STAT("Data collection ended successfully.  Exiting.");
				break;
			}
			else
			{
				LOG_DEBUG_STAT("Error in data collection process.  FAIL_HARD is false.  Looping.");
			}

			if(BBB_HVAC::GLOBALS::global_exit_flag == true)
			{
				LOG_INFO_STAT("Exiting because BBB_HVAC::GLOBALS::global_exit_flag is true.");
				break;
			}

			sleep(1);
		}
	}

	LOG_INFO_STAT("Application is exiting.");
}