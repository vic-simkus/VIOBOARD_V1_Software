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

#ifndef __HMI_DATA_LOGGER_CONFIG_HPP
#define __HMI_DATA_LOGGER_CONFIG_HPP

//#include "hmi_data_logger_context.hpp"
#include "lib/command_line_parms.h"

#include <string>
#include <memory>

#define CFG_CMDP_FAIL_HARD 		"--fail_hard"
#define CFG_CMDP_ROTATE_SIZE 	"--rotate_size"
#define CFG_CMDP_LOG_DIR 		"--log_dir"
#define CFG_CMDP_BASE_DATA_FILE "--base_data_file"
#define CFG_CMDP_MODE 			"--mode"
#define CFG_CMDP_PG_URL 		"--pg_url"
#define CFG_CMDP_PG_TEST 		"--pg_test"
#define CFG_CMDP_PID_FILE 		"--pid_file"

namespace HMI_DATA_LOGGER
{
	class Context;
	/**
	\brief Application configuration
	*/
	class Config
	{
		public:
			enum class MODE : char
			{
				NONE = 0,
				FILE,
				PGSQL,
				PFIELDS
			};

			static std::string mode_to_string( MODE _right ) {
				switch ( _right ) {
					case MODE::NONE:
						return "NONE";
						break;

					case MODE::FILE:
						return "FILE";
						break;

					case MODE::PGSQL:
						return "PGSQL";
						break;

					case MODE::PFIELDS:
						return"PFIELDS";
						break;
				}

				return "UNKNOWN";
			}
			/**
			\brief Default constructor.
			Initializes all of the instance values to their super reasonable defaults.
			*/
			Config();

			bool does_command_line_exist( const std::string& _name );
			std::string get_command_line_value( const std::string& _name );

			void set_command_line_parms( BBB_HVAC::COMMAND_LINE_PARMS* _parms );
			const BBB_HVAC::COMMAND_LINE_PARMS::EX_PARAM_LIST  get_command_line_parms( void ) const;
			const std::shared_ptr<BBB_HVAC::COMMAND_LINE_PARMS> get_command_line_processor( void ) const;

			size_t rotate_size;
			size_t current_file_index;

			std::string log_dir;
			std::string base_data_file_name;
			MODE mode;
			std::string pg_url;

			bool fail_hard;
			bool valid;

			std::string pid_file;

		protected:
			std::shared_ptr<BBB_HVAC::COMMAND_LINE_PARMS> command_line_parms;
	};

	bool create_configuration( Context& _ctx, int _argc, const char** _argv );
}

#endif
