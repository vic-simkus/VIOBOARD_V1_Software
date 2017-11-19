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


#ifndef SRC_INCLUDE_LIB_CONFIGURATOR_HPP_
#define SRC_INCLUDE_LIB_CONFIGURATOR_HPP_

#include "lib/logger.hpp"
#include "lib/hvac_types.hpp"

#include <string>
#include <vector>

#include <ostream>

using namespace std;

namespace BBB_HVAC
{

	class CONFIG_ENTRY
	{
	public:
		CONFIG_ENTRY(ENUM_CONFIG_TYPES _type, const CONFIG_PARTS_TYPE& _parts);
		~CONFIG_ENTRY();

		int get_part_as_int(size_t _idx) const throw(exception);
		double get_part_as_double(size_t _idx) const throw(exception);
		string get_part_as_string(size_t _idx) const throw(exception);

		/**
		 * Sets a specified part to the supplied value.
		 * \note The record type is not included in the part count.  For example, an SP record has two parts - label and value.  The 'SP' is not included in the part count.
		 */
		void set_part(size_t _idx, int _val) throw(exception);
		void set_part(size_t _idx, double _val) throw(exception);
		void set_part(size_t _idx, const string& _val) throw(exception);

		ENUM_CONFIG_TYPES get_type(void) const;
		bool get_is_dirty(void) const;

		string get_id(void) const;
		string get_id_for_human(void) const;

		string write_self_to_file(void) const throw(exception);

		size_t get_part_count(void) const;

		static string type_to_string(ENUM_CONFIG_TYPES _type) throw(exception);
		static ENUM_CONFIG_TYPES string_to_type(const string& _type) throw(exception);

	protected:
		ENUM_CONFIG_TYPES type;
		bool is_dirty;

		CONFIG_PARTS_TYPE parts;

	};

	class CONFIGURATOR
	{
	public:
		LOGGING::LOGGER __logger__;
		CONFIGURATOR(const string& _file);
		~CONFIGURATOR();
		void read_file(void) throw(exception);
		void write_file(void) const throw(exception);

		const CONFIG_TYPE_INDEX_TYPE& get_do_index(void) const;
		const CONFIG_TYPE_INDEX_TYPE& get_ai_index(void) const;
		const CONFIG_TYPE_INDEX_TYPE& get_sp_index(void) const;
		const CONFIG_TYPE_INDEX_TYPE& get_board_index(void) const;

		CONFIG_ENTRY& get_config_entry(size_t _idx)  throw(exception);

		const CONFIG_ENTRY& get_type_by_id(ENUM_CONFIG_TYPES _type,const string& _id)  throw(exception);

	protected:
		string file_name;
		string overlay_file_name;
		string cwd;

	private:

		void normalize_file_names(void) throw(exception);
		void check_file_permissions(void) throw(exception);
		void process_file(void) throw(exception);
		void process_line(size_t _line_idx) throw(exception);

		char* buffer;

		CONFIG_ENTRY_LIST_TYPE config_entries;

		CONFIG_TYPE_INDEX_TYPE do_configs;
		CONFIG_TYPE_INDEX_TYPE di_configs;
		CONFIG_TYPE_INDEX_TYPE ai_configs;
		CONFIG_TYPE_INDEX_TYPE ao_configs;
		CONFIG_TYPE_INDEX_TYPE sp_configs;
		CONFIG_TYPE_INDEX_TYPE board_configs;
	};
}

#endif /* SRC_INCLUDE_LIB_CONFIGURATOR_HPP_ */
