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

#ifndef SRC_INCLUDE_MESSAGE_LIB_HPP_
#define SRC_INCLUDE_MESSAGE_LIB_HPP_

#include "lib/message_types.hpp"
#include "lib/exceptions.hpp"
#include "lib/hvac_types.hpp"

#include <string>
#include <vector>
#include <memory>

#include <time.h>

namespace BBB_HVAC
{
	using namespace std;

	/**
	 * A message used to communicated between nodes.
	 */
	class MESSAGE
	{
		public:
			/**
			 * Constructor.
			 * \param _type Message type
			 * \param _payload A vector of strings of the parts/payload of the message
			 */
			MESSAGE( const MESSAGE_TYPE& _type, const vector<string>& _payload = vector<string>() );

			/**
			 * Destructor
			 */
			~MESSAGE();

			/**
			 * Returns the message type
			 * \return Message type
			 */
			MESSAGE_TYPE get_message_type( void ) const;

			/**
			 * Returns the message payload length as it will be sent or was received.
			 * \return Total message length including the terminating new line
			 */
			size_t get_length( void ) const;

			/**
			Returns the number of parts in the message's payload.
			*/
			size_t get_part_count( void ) const;

			/**
			 * Returns the message payload as it will be sent or was received.
			 * \return Payload including the terminating new line.
			 */
			const string& get_payload( void ) const;

			/**
			 * Returns the parsed message parts/payload.
			 * \return Message parts/payload as a vector of strings.
			 */
			const vector<string>& get_parts( void ) const;

			/**
			 * Tags the message as being received.  Calling this method more than once will raise a runtime_exception
			 */
			void tag_received( void ) ;

			/**
			 * Tags the message as being sent.  Calling this method more than once will raise a runtime_exception
			 */
			void tag_sent( void ) ;

			/**
			 * Generates a string that is intended to be used for human consumption for debugging purposes.  The format is not guaranteed to be stable.
			 * \return A string containing debuging information intended for human consumption.
			 */
			string to_string( void ) const;

			/**
			 * Gets a message part as an unsigned integer.
			 * \param _part Index of the part to convert to a number.
			 * \return Value of the part as an unsigned integer.  Throws an exception if the part payload can not be parsed into a numerical form.
			 */
			uint16_t get_part_as_ui( size_t _part ) ;
			/**
			 * Gets a message part as a signed integer.
			 * \see get_par_as_ui(_part)
			 */
			int16_t get_part_as_si( size_t _part ) ;

			/**
			 * Gets a message part as a string
			 * \return Part as a string.
			 */
			string get_part_as_s( size_t _part ) ;
			/**
			 * Gets a message part as a double
			 * \return Part as a string.
			 */
			double get_part_as_d( size_t _part ) ;

			/**
			 * Gets the timestamp of when this message instance was sent to remote.
			 * \return Timestamp of the event.
			 */
			const timespec* get_message_sent_timestamp( void ) const;

			/**
			 * Gets the timestamp of when this message instance was created.
			 * \return Timestamp of the event.
			 */
			const timespec* get_message_created_timestamp( void ) const;

			/**
			 * Gets the timestamp of when this message was received.
			 * \return Timestamp of the event.
			 */
			const timespec* get_message_received_timestamp( void ) const;

			//void trim_message( void );

			/**
			 * Separating character that is used to create or parse message payloads.
			 */
			static const char sep_char;

			static void message_to_map( const MESSAGE_PTR& _message, std::map<std::string, std::string>& _dest_map ) ;

		protected:
			/**
			 * Checks to see if the supplied part index is valid (is in range)
			 * \param _part Part index.
			 */
			void check_part_index( size_t _part ) ;

			/**
			 * Protected initializer.  Zeros out and resets all of the classe's properties.
			 */
			void init( void );

			/**
			 * Builds the messages payload.
			 */
			void build_message( void );

			/**
			 * Message type.
			 */
			MESSAGE_TYPE message_type;

			/**
			 * Length of the payload including the terminating new line.
			 */
			size_t length;

			/**
			 * Raw payload.
			 */
			string payload;

			/**
			 * Parsed parts of the message.
			 */
			vector<string> parts;

			/**
			 * Timestamp of when the class was instantiated.
			 */
			struct timespec* class_created;

			/**
			 * Timestamp of when this message was received from remote.
			 */
			struct timespec* message_received;

			/**
			 * Timestamp of when this message was sent to remote.
			 */
			struct timespec* message_sent;

			/**
			 * Gets the current timestamp and places the supplied buffer.
			 * \param _tm Destination buffer.
			 */
			static void get_timestamp( timespec* _tm ) ;
	};
}

#endif /* SRC_INCLUDE_MESSAGE_LIB_HPP_ */
