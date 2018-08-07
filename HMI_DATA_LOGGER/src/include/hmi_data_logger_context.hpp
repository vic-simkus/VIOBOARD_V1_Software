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

#ifndef __HMI_DATA_LOGGER_HPP
#define __HMI_DATA_LOGGER_HPP

#include "hmi_data_logger_config.hpp"
#include "lib/logger.hpp"

#include <fstream>

namespace HMI_DATA_LOGGER
{
	/**
	\brief Applications context.
	Basically all of the stuff that would be in the global context is put here.
	*/
	class HMI_DATA_LOGGER_CONTEXT
	{
	public:
		DEF_LOGGER;

		HMI_DATA_LOGGER_CONTEXT()
		{
			INIT_LOGGER( "HMI_DATA_LOGGER_CONTEXT" );
			return;
		}

		/**
		Checks to see if the specified data output directory is actually ... a directory and so forth.
		This method does reset this->full_data_dir.  If the method returns false this->full_data_dir remains empty.
		\return True if the check is successful.  Also sets this->full_data_dir.  False otherwise and clears this->full_data_dir.
		*/
		bool check_data_dir( void );

		/**
		Goes through all of the directory entries in the log_dir and guesses the next file index.
		*/
		size_t get_next_data_file_index( void ) throw( std::runtime_error );

		std::string get_data_file_name_before_index( void );

		std::string get_data_file_name_after_index( void );

		std::string get_next_data_file_name(void);

		void set_prog_name( const std::string& _p ) throw( std::logic_error );

		void set_prog_name_fixed( const std::string& _p ) throw( std::logic_error );

		std::string get_full_data_dir(void) const;

		std::ofstream& get_output_stream(void);

		void open_output_stream(void) throw (exception);

		void close_output_stream(void);

		bool get_new_file_flag(void);
		void reset_new_file_flag(void);


		HMI_DATA_LOGGER_CONFIG configuration;

	protected:

		std::string current_data_file_name;
		std::string prog_name;
		std::string prog_name_fixed;
		std::string full_data_dir;

		std::ofstream output_stream;

		bool new_file_flag;

	};	// END HMI_DATA_LOGGER_CONTEXT

}

#endif