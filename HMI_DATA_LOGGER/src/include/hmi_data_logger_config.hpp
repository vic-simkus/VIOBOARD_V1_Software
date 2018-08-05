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

#ifndef __HMI_DATA_LOGGER_CONFIG_HPP
#define __HMI_DATA_LOGGER_CONFIG_HPP

#include <string>

namespace HMI_DATA_LOGGER
{
	/**
	\brief Application configuration
	*/
	class HMI_DATA_LOGGER_CONFIG
	{
	public:
		/**
		\brief Default constructor.
		Initializes all of the instance values to their super reasonable defaults.
		*/
		HMI_DATA_LOGGER_CONFIG()
		{
			this->rotate_size = 0;
			this->log_dir = "./log_data";
			this->base_data_file_name = "sys_status_log.#.csv";
			this->current_file_index = 0;
			this->fail_hard = true;
		}

		 size_t rotate_size;
		 std::string log_dir;
		 std::string base_data_file_name;
		 bool fail_hard;
		 size_t current_file_index;
	};
}
#endif
