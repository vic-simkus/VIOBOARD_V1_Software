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
#include <map>

using namespace std;

/*
XXX - Create a mechanism of valid and invalid instances of BOARD_POINT and SET_POINT.  I.E. if an instance was fully created or it's just a partial instance such as when it was
serialized/deserialized.
*/
namespace BBB_HVAC
{

	/**
	 * Types of configuration elements
	 */
	enum class ENUM_CONFIG_TYPES : unsigned int
	{
		INVALID = 0, 	/// Invalid value.
		DO,				/// Digital output.  A digital output on an IO board.
		AI,				/// Analog input.  An analog input on an IO board.
		SP,				/// Set-point.  A value that the system will try to achieve.  Meaning of setpoint is based on context.
		BOARD,			/// IO Board entry.  Defines an IO board and its communication port.
		MAP				/// Mapping entry.
	};

	/**
	CT (Configuration Type) BOARD field indexes.  Every BOARD configuration entry needs to have the following fields in the specified order..
	*/
	enum class ENUM_CT_BOARD_IDX : unsigned int
	{
		TAG,			/// Tag/ID of the board.
		DEVICE,			/// Linux serial device name.
		OPTIONS,		/// Options.
	};

	/**
	CT (Configuration Type) DO field indexes.  Every DO (digital output) configuration entry needs to have the following fields in the specified order.
	*/
	enum class ENUM_CT_POINT_IDX : unsigned int
	{
		BOARD_TAG,		/// Parent board tag/ID.
		INDEX,			/// Board input index.
		DESCRIPTION		/// Description of the output.
	};

	/**
	CT (Configuration Type) SP field indexes.  Every SP (set point) configuration entry needs to  have the following fields in the specified order.
	*/
	enum class ENUM_CT_SP_IDX : unsigned int
	{
		DESCRIPTION,	/// Set point description.
		VALUE			/// Set point value.
	};


	/**
	 * Operator to turn an ENUM_CONFIG_TYPES into a string number
	 * \param os  Output stream
	 * \param _v ENUM_CONFIG_TYPES instance
	 * \return Output stream instance
	 */
	inline std::ostream& operator<< ( std::ostream& os, ENUM_CONFIG_TYPES _v )
	{
		return os << static_cast < unsigned int >( _v );
	}

	/**
	 * Type for a list of configuration parts.
	 */
	typedef vector<string> CONFIG_PARTS_TYPE;

	/**
	 * Forward declaration
	 */
	class CONFIG_ENTRY;

	/**
	 * Type for list of configuration entries
	 */
	typedef vector<CONFIG_ENTRY> CONFIG_ENTRY_LIST_TYPE;

	/**
	 * Type for list of configuration type indexes
	 */
	typedef vector<size_t> CONFIG_TYPE_INDEX_TYPE;

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

		bool get_part_as_bool(size_t _idx) const throw (exception);

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
		std::string to_string(void) const;

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
		inline SET_POINT()
		{
			this->description.clear();
			this->value = 0xFFFF;
			this->index = 0xFF;
		}

		inline SET_POINT( const std::string& _description, double _value)
		{
			this->description = _description;
			this->value = _value;
			this->index = 0xFE;

			return;
		}

		inline SET_POINT( const std::string& _description, double _value, size_t _index ) {
			this->description = _description;
			this->value = _value;
			this->index = _index;
			return;
		}

		inline SET_POINT( const SET_POINT& _src ) {
			this->description = _src.description;
			this->value = _src.value;
			this->index = _src.index;
			return;
		}

		inline SET_POINT( const CONFIG_ENTRY& _config_entry, size_t _index ) {
			this->description = _config_entry.get_part_as_string( 0 );
			this->value = _config_entry.get_part_as_double( 1 );
			this->index = _index;
			return;
		}

		inline std::string to_string( void ) const {
			return "(" + CONFIG_ENTRY::type_to_string( ENUM_CONFIG_TYPES::SP ) + ',' + this->description + ',' + num_to_str( this->value ) + ")";
		}

		static inline SET_POINT from_string(const std::string& _source)
		{
			/*
			No fucking error checking at all...

			XXX - need to make this not shit.  Like add error checking and handling and other such luxuries.
			*/

			// Get rid of the perens.
			std::string str = trimmed(_source);
			str = str.substr(1,_source.length()-2);


			std::vector<std::string> parts;

			split_string_to_vector(str,',',parts);

			if(parts.size() != 3)
			{
				return SET_POINT();
			}

			return SET_POINT(parts[1],stod(parts[2]));
		}

		/*
		static inline std::string to_string_static( const SET_POINT& _point ) {
			return _point.to_string();
		}
		*/

		static inline std::string to_string_static(const std::pair<std::string, BBB_HVAC::SET_POINT>& _pair)
		{
			return _pair.second.to_string();
		}

		inline double get_value(void) const
		{
			return this->value;
		}

		inline void set_value(double _val)
		{
			this->value = _val;
		}

		inline size_t get_index(void) const
		{
			return this->index;
		}

		inline std::string get_description(void) const
		{
			return description;
		}

	protected:
		std::string description;
		double value;
		size_t index;
	private:
	};

	typedef std::map<std::string,SET_POINT> SET_POINT_MAP;

	class BOARD_POINT
	{
	public:

		inline BOARD_POINT()
		{
			this->board_tag.clear();
			this->point_id = 0xFF;
			this->description.clear();
			this->type = ENUM_CONFIG_TYPES::INVALID;
			this->index = 0;

			this->ai_type = AI_TYPE::INVALID;
			this->min = 0;
			this->max = 0;

			return;
		}

		/*

		I guess we're not using this method anywhere... VS - 07/29/18

		inline BOARD_POINT( const std::string& _board_tag, unsigned char _point_id, const std::string& _description, ENUM_CONFIG_TYPES _type, size_t _index ) {
			this->board_tag = _board_tag;
			this->point_id = _point_id;
			this->description = _description;
			this->type = _type;
			this->index = _index;

			this->ai_type = AI_TYPE::INVALID;
			this->min = 0;
			this->max = 0;

			return;
		}*/

		inline BOARD_POINT( const BOARD_POINT& _src ) {
			this->board_tag = _src.board_tag;
			this->point_id = _src.point_id;
			this->description = _src.description;
			this->type = _src.type;
			this->index = _src.index;

			this->ai_type = _src.ai_type;
			this->min = _src.min;
			this->max = _src.max;

			this->is_celcius = _src.is_celcius;

			return;
		}

		inline BOARD_POINT( const CONFIG_ENTRY& _config_entry, ENUM_CONFIG_TYPES _type, size_t _index )
		{
			this->board_tag = _config_entry.get_part_as_string( 0 );
			this->point_id = _config_entry.get_part_as_uchar( 1 );
			this->description = _config_entry.get_part_as_string( 2 );
			this->type = _type;
			this->index = _index;
			this->is_celcius = false;

			if(this->type == ENUM_CONFIG_TYPES::AI)
			{
				if(_config_entry.get_part_as_string(3) == "420")
				{
					this->ai_type = AI_TYPE::CL_420;

					this->min = _config_entry.get_part_as_int(4);
					this->max = _config_entry.get_part_as_int(5);
				}
				else if (_config_entry.get_part_as_string(3) == "ICTD")
				{
					this->ai_type = AI_TYPE::ICTD;
					this->is_celcius = (_config_entry.get_part_as_string(4) == "C") ? true : false;
				}

				//
				// Yeah fudge error handling a bit.
				//
			}

			return;
		}

		inline ~BOARD_POINT() {
			this->board_tag.clear();
			this->point_id = 0xFF;
			this->description.clear();
			this->type = ENUM_CONFIG_TYPES::INVALID;
			this->index = 0;

			this->ai_type = AI_TYPE::INVALID;
			this->min = 0;
			this->max = 0;

			this->is_celcius = true;

			return;
		}

		const std::string& get_board_tag( void ) const {
			return this->board_tag;
		}

		const std::string& get_description( void ) const {
			return this->description;
		}

		unsigned char get_point_id( void ) const {
			return this->point_id;
		}

		ENUM_CONFIG_TYPES get_type( void ) const {
			return this->type;
		}

		size_t get_index( void ) const {
			return this->index;
		}

		inline std::string to_string( void ) const {
			return "(" + CONFIG_ENTRY::type_to_string( this->type ) + ',' + this->board_tag + ',' + num_to_str( this->point_id ) + ',' + this->description + ")";
		}

		static inline BOARD_POINT from_string(const std::string& _source)
		{
			/*
			No fucking error checking at all...

			XXX - need to make this not shit.  Like add error checking and handling and other such luxuries.
			*/

			// Get rid of the perens.
			std::string str = trimmed(_source);
			str = str.substr(1,_source.length()-2);

			std::vector<std::string> parts;

			split_string_to_vector(str,',',parts);

			BOARD_POINT ret;

			if(parts.size() != 4)
			{
				return ret;
			}

			ret.type = CONFIG_ENTRY::string_to_type(parts[0]);
			ret.board_tag = parts[1];
			ret.point_id = (unsigned char)stoi(parts[2]);
			ret.description = parts[3];

			return ret;
		}

		static inline std::string to_string_static( const BOARD_POINT& _point ) {
			return _point.to_string();
		}

		long get_min_value(void) const
		{
			return this->min;
		}

		long get_max_value(void) const
		{
			return this->max;
		}

		bool get_is_celcius(void) const
		{
			return this->is_celcius;
		}

		const AI_TYPE& get_ai_type(void) const
		{
			return this->ai_type;
		}

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

	private:

		friend bool operator==(const BOARD_POINT&,const BOARD_POINT& );
	};

	inline bool operator==(const BOARD_POINT& _l, const BOARD_POINT& _r)
	{
		if(_l.type == _r.type && _l.board_tag == _r.board_tag && _l.point_id == _r.point_id)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	typedef std::vector<BOARD_POINT> BOARD_POINT_VECTOR;

	/**
	A class for reading/writing a configuration file.
	*/
	class CONFIGURATOR
	{
	public:
		/**
		Instance logger.
		*/
		LOGGING::LOGGER __logger__;

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
		inline BOARD_POINT_VECTOR get_do_points( void ) const {
			return this->do_points;
		}

		/**
		Returns the AI (analog input) configurations entries in the instance.
		*/
		inline BOARD_POINT_VECTOR get_ai_points( void ) const {
			return this->ai_points;
		}

		/**
		Returns the SP (set ppoint) configuration entires in the instance.  The key is the label of the set point.
		*/
		inline const SET_POINT_MAP& get_sp_points( void ) const {
			return this->sp_points;
		}

		/**
		Returns the value of the specified set point.  An exception is thrown if a set point with the specified name does not exist.
		*/
		inline double get_sp_value(const string& _name) const throw (exception)
		{
			return this->sp_points.at(_name).get_value();
		}

		/**
		Sets the value of the specified set point.  An exception is thrown if a set point with the specified name does not exist.
		*/
		inline void set_sp_value(const string& _name,double _value) throw (exception)
		{
			SET_POINT & sp = this->sp_points.at(_name);
			sp.set_value(_value);
			CONFIG_ENTRY & ce = this->config_entries.at(sp.get_index());
			ce.set_part(1,_value);
			return;
		}

		/**
		Returns the point map.
		\see CONFIGURATOR::point_map
		*/
		inline const std::map<std::string,BOARD_POINT> & get_point_map(void) const
		{
			return this->point_map;
		}

	protected:
		string file_name;
		string overlay_file_name;
		string cwd;

	private:

		void normalize_file_names( void ) throw( exception );
		void check_file_permissions( void ) throw( exception );
		void process_file( void ) throw( exception );
		void process_line( size_t _line_idx ) throw( exception );
		void process_mapping(const CONFIG_ENTRY& _ce) throw (exception);

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
		std::map<std::string,BOARD_POINT> point_map;
	};
}

#endif /* SRC_INCLUDE_LIB_CONFIGURATOR_HPP_ */
