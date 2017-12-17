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


#ifndef SRC_INCLUDE_LIB_EXCEPTIONS_HPP_
#define SRC_INCLUDE_LIB_EXCEPTIONS_HPP_

#include <stdexcept>
#include <string>
#include "lib/string_lib.hpp"

using namespace std;

namespace BBB_HVAC
{

#define THROW_EXCEPTION(type,msg) throw type(string(__PRETTY_FUNCTION__) + "@" + num_to_str(__LINE__) + ": " + msg)

	/**
	 * Utility function to create an std::string out of the system error string.
	 * \param _msg Message to prepend to the error string.
	 * \return A string instance describing the last errno error.
	 */
	extern std::string create_perror_string( const string& _msg );

	/**
	 * Exceptions used by the application
	 */
	namespace EXCEPTIONS
	{

		/**
		 * Low level network error.
		 */
		class NETWORK_ERROR : public runtime_error
		{
		public:

			/**
			 * Constructor.
			 * \param _what Description of the error.
			 */
			inline NETWORK_ERROR( const string& _what ) :
				runtime_error( _what ) {
				return;
			}
		};

		/**
		 * Connection error.  Will be thrown if the initial connection to the server process fails.
		 */
		class CONNECTION_ERROR : public NETWORK_ERROR
		{
		public:

			/**
			 * Constructor.
			 * \param _what Description of the error.
			 */
			inline CONNECTION_ERROR( const string& _what ) :
				NETWORK_ERROR( _what ) {
				return;
			}
		};

		/**
		 * Protocol error.  Will be thrown if the connection is established, but some sort of communication error is encountered.
		 */
		class PROTOCOL_ERROR : public NETWORK_ERROR
		{
		public:

			/**
			 * Constructor.
			 * \param _what Description of the error.
			 */
			inline PROTOCOL_ERROR( const string& _what ) :
				NETWORK_ERROR( _what ) {
				return;
			}
		};

		class MESSAGE_ERROR : public runtime_error
		{
		public:

			inline MESSAGE_ERROR( const string& _what ) :
				runtime_error( _what ) {
				return;
			}
		};

		class MESSAGE_OVERFLOW : public MESSAGE_ERROR
		{
		public:

			inline MESSAGE_OVERFLOW( const string& _what ) :
				MESSAGE_ERROR( _what ) {
				return;
			}
		};

		class MESSAGE_UNDERFLOW : public MESSAGE_ERROR
		{
		public:

			inline MESSAGE_UNDERFLOW( const string& _what ) :
				MESSAGE_ERROR( _what ) {
				return;
			}
		};

		class LOCK_ERROR : public runtime_error
		{
		public:

			inline LOCK_ERROR( const string& _what ) : runtime_error( _what ) {
				return;
			}
		};
	}
}

#endif /* SRC_INCLUDE_LIB_EXCEPTIONS_HPP_ */
