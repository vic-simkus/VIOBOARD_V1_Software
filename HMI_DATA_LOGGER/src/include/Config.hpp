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
			Config() {
				this->rotate_size = 0;
				this->log_dir = "./log_data";
				this->base_data_file_name = "sys_status_log.#.csv";
				this->current_file_index = 0;
				this->fail_hard = true;
				this->mode = MODE::NONE;
				this->valid = false;

				return;
			}

			size_t rotate_size;
			size_t current_file_index;

			std::string log_dir;
			std::string base_data_file_name;
			MODE mode;
			std::string pg_url;

			bool fail_hard;
			bool valid;

			std::shared_ptr<BBB_HVAC::COMMAND_LINE_PARMS> command_line_parms;
	};

	bool create_configuration( Context& _ctx, int _argc, const char** _argv );
}

#endif
