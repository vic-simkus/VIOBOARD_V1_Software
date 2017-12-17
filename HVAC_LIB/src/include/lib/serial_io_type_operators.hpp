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

#ifndef SERIAL_IO_TYPE_OPERATORS_HPP
#define SERIAL_IO_TYPE_OPERATORS_HPP

#include <string>
#include <ostream>

/**
 * \file Various operators for use with the serial comm stuff to make our lives easier.  Mostly stuff to convert data types to strings.
 */
namespace BBB_HVAC
{
	using namespace EXCEPTIONS;

	namespace IOCOMM
	{

		/**
		 * \brief Operator to dump a token into a stream.  Converts the token into a human-readable representation and dumps it into the output stream.
		 * \param os Target output stream.
		 * \param pair Source pair.
		 * \return A reference to the supplied output stream.
		 */
		inline std::ostream& operator<< ( std::ostream& os, const token& pair )
		{
			return os << "(" << pair.first << "," << pair.second << ")";
		}

		/**
		 * \brief An operator to concatenate a string and a string representation of a token.  Used for debugging and such.
		 * \param _left  Thing left of the operator.
		 * \param pair Thing right of the operator.
		 * \return A string concatenation.
		 */
		inline std::string operator+ ( const char* _left, const token& pair )
		{
			return std::string( _left ) + "(" + std::to_string( pair.first ) + "," + std::to_string( pair.second ) + ")";
		}

		/**
		 * \brief An operator to dump a cache entry into a stream.  Converts the instance into a human-readable format and dumps it into the output stream.
		 * \param os Output stream.
		 * \param _cab Cache entry
		 * \return A reference to the same output stream.
		 */
		inline std::ostream& operator<< ( std::ostream& os, const CACHE_ENTRY_BASE& _cab )
		{
			return os << _cab.to_string();
		}

		/**
		 * \brief An operator to concatenate a string an a string representation of a cache entry.  Used for debugging and such.
		 * \param _left  Thing left of the operator.
		 * \param _cab  Thing right of the operator.
		 * \return A string concatenation.
		 */
		inline std::string operator+ ( const char* _left, const CACHE_ENTRY_BASE& _cab )
		{
			return std::string( _left ) + _cab.to_string();
		}
	}
}

#endif /* SERIAL_IO_TYPE_OPERATORS_HPP */

