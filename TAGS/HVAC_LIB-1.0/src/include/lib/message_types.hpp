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

#ifndef MESSAGET_TYPES_C_
#define MESSAGET_TYPES_C_

#include "lib/hvac_types.hpp"

#include <string>
#include <memory>
#include <map>
#include <sstream>
#include <ostream>

namespace BBB_HVAC
{

	/**
	 * Class representing a message type supported.
	 * This class is not intended to be used directly.  It is intended to be wrapped in a std::shared_ptr template.
	 * \see MESSAGE_TYPE
	 */
	class __MESSAGE_TYPE
	{
	public:
		/**
		 * Constructor
		 * \param _type Message type.
		 * \param _label Human readable label describing the type
		 */
		__MESSAGE_TYPE( ENUM_MESSAGE_TYPE _type, const std::string& _label );

		/**
		 * Produces a human readable description of the message type.
		 * Intended to be used for debugging.
		 * \return A string containing the numerical value and label of the type.  The returned format is not guaranteed to be stable.
		 */
		inline std::string to_string( void ) const {
			std::stringstream ret;
			ret << "[" << this->type << "][" << label << "]";
			return ret.str();
		}

		/**
		 * Helper operator to enable dumping the to_string value to an std::ostream.
		 */
		inline std::ostream& operator<< ( std::ostream& str ) {
			str << this->to_string();
			return str;
		}

		/**
		 * Numeric type.
		 */
		ENUM_MESSAGE_TYPE type;

		/**
		 * Label intended for human consumption.
		 */
		std::string label;
	};

	/*
	 * Forward deceleration.  Like duh
	 */
	class MESSAGE_TYPE_MAPPER;

	/**
	 * A class wrapper for static message type definition.
	 * This class exists to enable us to initialize static data at run time.
	 */
	class __MESSAGE_TYPES_INT
	{
		friend MESSAGE_TYPE_MAPPER;
	public:
		/**
		 * Constructor.
		 */
		__MESSAGE_TYPES_INT();

	protected:

		/**
		 * An enum to message type map instance.
		 */
		MAP_TYPE_TO_LABEL enum_to_type;

		/**
		 * A label to message type map instance.
		 */
		MAP_LABEL_TO_TYPE label_to_type;

	};

	/**
	 * A static class that provides access to the message types supported by the system.
	 * This class is not intended to be manually instantiated.
	 */

	class MESSAGE_TYPE_MAPPER
	{
	public:
		/**
		 * Gets a the number of message types supported by the system.
		 * \return Number of message types supported by the system.
		 */
		static size_t get_message_type_count( void );

		/**
		 * Maps a label to a type.
		 * \param _label Type label.
		 * \return A MESSAGE_TYPE instance for the supplied label.
		 */
		static MESSAGE_TYPE get_message_type_by_label( const std::string& _label );

		/**
		 * Maps an enum to a type.
		 * \param _enum Type enum.
		 * \return A MESSAGE_TYPE instance for the supplied enum.
		 */
		static MESSAGE_TYPE get_message_type_by_enum( const ENUM_MESSAGE_TYPE& _enum );

		/**
		 * Dumps a pretty list of supported message types intended for human construction to a stream.
		 * The format is not guaranteed to to be stable.
		 * \param out The output stream that the text should be dumped to.
		 */
		static void dump_supported_messages( std::ostream& out );

	private:

		/**
		 * Hidden constructor.
		 */
		inline MESSAGE_TYPE_MAPPER() {
			return;
		}

		/**
		 * Internal mapper.
		 */
		static __MESSAGE_TYPES_INT __internal_mapper;
	};
}
#endif /* MESSAGET_TYPES_C_ */
