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
#include "lib/configurator/configurator_types.hpp"

#include <string>
#include <vector>
#include <ostream>
#include <map>

using namespace std;

/*
XXX - Create a mechanism of valid and invalid instances of BOARD_POINT and SET_POINT.  I.E. if an instance was fully created or it's just a partial instance such as when it was
serialized/deserialized.
*/
namespace BBB_HVAC
{
	/**
	A singular line in a configuration file is a 'CONFIG_ENTRY' instance.
	*/
	class CONFIG_ENTRY
	{
		public:
			/**
			Constructor.
			\param _type Type of configuration entry.
			\param _parts A list of parts.
			*/
			CONFIG_ENTRY( ENUM_CONFIG_TYPES _type, const CONFIG_PARTS_TYPE& _parts );

			/*
			Destructor.
			*/
			~CONFIG_ENTRY();

			/**
			Returns a specified part as an integer.  If the conversion fails an exception is thrown.
			*/
			int get_part_as_int( size_t _idx ) const throw( exception );

			/**
			Returns a specified part as an unsigned character.  If the conversion fails an exception is thrown.
			*/
			unsigned char get_part_as_uchar( size_t _idx ) const throw( exception );

			/**
			Returns a specified part as a double.  If the conversion fails an exception is thrown.
			*/
			double get_part_as_double( size_t _idx ) const throw( exception );

			/**
			Returns a specified part as a string.  If the index is invalid an exception is thrown.
			*/
			string get_part_as_string( size_t _idx ) const throw( exception );

			bool get_part_as_bool( size_t _idx ) const throw( exception );

			/**
			 * Sets a specified part to the supplied value.
			 * \note The record type is not included in the part count.  For example, an SP record has two parts - label and value.  The 'SP' is not included in the part count.
			 */
			void set_part( size_t _idx, int _val ) throw( exception );
			void set_part( size_t _idx, double _val ) throw( exception );
			void set_part( size_t _idx, const string& _val ) throw( exception );


			/**
			Returns the type of the current configuration entry.
			*/
			ENUM_CONFIG_TYPES get_type( void ) const;

			/**
			Returns true if this instance has been modified since instantiation.
			*/
			bool get_is_dirty( void ) const;

			/**
			Returns the string representation of this instance suitable to be written to a file.
			*/
			string write_self_to_file( void ) const throw( exception );

			/**
			Returns the number of parts in this entry.  The record type is not included in the part count.
			*/
			size_t get_part_count( void ) const;

			/**
			Returns the string representation of the instance.
			*/
			std::string to_string( void ) const;

			/**
			Converts the type enum to a string suitable for outputting to a configuration file.  An exception is thrown in case of hinkiness.
			*/
			static string type_to_string( ENUM_CONFIG_TYPES _type ) throw( exception );

			/**
			Converts a string representation of an entry type to an enum.  An exception is thrown if the conversion can not be made.
			*/
			static ENUM_CONFIG_TYPES string_to_type( const string& _type ) throw( exception );

		protected:
			/**
			Type of the current instance.
			*/
			ENUM_CONFIG_TYPES type;

			/**
			Has the instance been modified since creation.
			\see get_is_dirty
			*/
			bool is_dirty;

			/**
			A list of parts for this entry.
			*/
			CONFIG_PARTS_TYPE parts;
	};

	class SET_POINT
	{
		public:
			SET_POINT();
			SET_POINT( const std::string& _description, double _value );
			SET_POINT( const std::string& _description, double _value, size_t _index );
			SET_POINT( const SET_POINT& _src );
			SET_POINT( const CONFIG_ENTRY& _config_entry, size_t _index );
			std::string to_string( void ) const;
			static  SET_POINT from_string( const std::string& _source );
			static  std::string to_string_static( const std::pair<std::string, BBB_HVAC::SET_POINT>& _pair );
			double get_value( void ) const;
			void set_value( double _val );
			size_t get_index( void ) const;
			std::string get_description( void ) const;

		protected:
			std::string description;
			double value;
			size_t index;
		private:
	};


	class BOARD_POINT
	{
		public:

			BOARD_POINT();
			BOARD_POINT( const BOARD_POINT& _src );
			BOARD_POINT( const CONFIG_ENTRY& _config_entry, ENUM_CONFIG_TYPES _type, size_t _index );
			~BOARD_POINT();
			const std::string& get_board_tag( void ) const;
			const std::string& get_description( void ) const;
			unsigned char get_point_id( void ) const;
			ENUM_CONFIG_TYPES get_type( void ) const;
			size_t get_index( void ) const;
			std::string to_string( void ) const;
			static  BOARD_POINT from_string( const std::string& _source );
			static  std::string to_string_static( const BOARD_POINT& _point );
			long get_min_value( void ) const;
			long get_max_value( void ) const;
			bool get_is_celcius( void ) const;
			const AI_TYPE& get_ai_type( void ) const;

		protected:
			unsigned char point_id;
			std::string board_tag;
			std::string description;
			ENUM_CONFIG_TYPES type;

			AI_TYPE ai_type;
			long min;
			long max;

			bool is_celcius;

			size_t index;

			friend bool operator==( const BOARD_POINT&, const BOARD_POINT& );
		private:


	};

	/**
	A class for reading/writing a configuration file.
	*/
	class CONFIGURATOR
	{
		public:
			/**
			Instance logger.
			*/
			DEF_LOGGER;

			/**
			Constructor.
			*/
			CONFIGURATOR( const string& _file );

			/**
			Destructor.
			*/
			~CONFIGURATOR();

			void read_file( void ) throw( exception );

			void write_file( void ) const throw( exception );

			const CONFIG_TYPE_INDEX_TYPE& get_board_index( void ) const;

			/**
			Gets a config entry by a global config entry index.
			\param _idx Global config entry index.
			*/
			CONFIG_ENTRY& get_config_entry( size_t _idx )  throw( exception );

			const CONFIG_ENTRY& get_type_by_id( ENUM_CONFIG_TYPES _type, const string& _id )  throw( exception );

			/**
			Returns the DO (digital output) configuration entries in the instance.
			*/
			BOARD_POINT_VECTOR get_do_points( void ) const;


			/**
			Returns the AI (analog input) configurations entries in the instance.
			*/
			BOARD_POINT_VECTOR get_ai_points( void ) const;


			/**
			Returns the SP (set ppoint) configuration entires in the instance.  The key is the label of the set point.
			*/
			const SET_POINT_MAP& get_sp_points( void ) const;


			/**
			Returns the value of the specified set point.  An exception is thrown if a set point with the specified name does not exist.
			*/
			double get_sp_value( const string& _name ) const throw( exception );


			/**
			Sets the value of the specified set point.  An exception is thrown if a set point with the specified name does not exist.
			*/
			void set_sp_value( const string& _name, double _value ) throw( exception );


			/**
			Returns the point map.
			\see CONFIGURATOR::point_map
			*/
			const std::map<std::string, BOARD_POINT>& get_point_map( void ) const;

		protected:
			string file_name;
			string overlay_file_name;
			string cwd;

		private:

			void normalize_file_names( void ) throw( exception );
			void check_file_permissions( void ) throw( exception );
			void process_file( void ) throw( exception );
			void process_line( size_t _line_idx ) throw( exception );
			void process_mapping( const CONFIG_ENTRY& _ce ) throw( exception );

			char* buffer;

			CONFIG_ENTRY_LIST_TYPE config_entries;

			BOARD_POINT_VECTOR do_points;
			BOARD_POINT_VECTOR ai_points;
			SET_POINT_MAP sp_points;

			CONFIG_TYPE_INDEX_TYPE board_configs;
			CONFIG_TYPE_INDEX_TYPE map_configs;

			/**
			Mapping between 'globally unique' point names and the actual points.
			The key is the globally unique name.  The payload is an instance of the BOARD_POINT.
			\see Configuration directive 'MAP'
			*/
			std::map<std::string, BOARD_POINT> point_map;
	};

	bool operator==( const BOARD_POINT& _l, const BOARD_POINT& _r );
	/**
	 * Operator to turn an ENUM_CONFIG_TYPES into a string number
	 * \param os  Output stream
	 * \param _v ENUM_CONFIG_TYPES instance
	 * \return Output stream instance
	 */
	std::ostream& operator<< ( std::ostream& os, ENUM_CONFIG_TYPES _v );
}

#endif /* SRC_INCLUDE_LIB_CONFIGURATOR_HPP_ */
