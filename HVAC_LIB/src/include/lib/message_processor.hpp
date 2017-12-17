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


#ifndef SRC_INCLUDE_LIB_MESSAGE_PROCESSOR_HPP_
#define SRC_INCLUDE_LIB_MESSAGE_PROCESSOR_HPP_

#include "lib/message_lib.hpp"
#include "lib/exceptions.hpp"
#include "lib/configurator.hpp"
#include "lib/hvac_types.hpp"

#include <vector>

#include <pthread.h>

namespace BBB_HVAC
{
	/**
	 * Stuff internal to the message processor
	 */
	namespace MSG_PROC
	{

		/**
		 * A wrapper around a std::vector.  Wraps the template nastiness and provides basic convenience methods.
		 */
		class MESSAGE_QUEUE
		{
		public:
			/**
			 * Constructor.
			 * \param _size Size of the queue.
			 */
			MESSAGE_QUEUE( unsigned int _size );
			/**
			 * Destructor.
			 */
			~MESSAGE_QUEUE();

			/**
			 * Adds a message to the queue.
			 * Depending on the _mode parameter, the method may throw an exception or delete the oldest message in case the queue is full.
			 * \param _message Message to be added.
			 * \param _mode Action to take if the queue is full
			 * \see APPEND_MODE
			 */
			void add_message( MESSAGE_PTR& _message, ENUM_APPEND_MODE _mode = ENUM_APPEND_MODE::LOSE_OVERFLOW ) throw( exception );

			/**
			 * Returns the number of messages in the queue.
			 * \return Number of messages in the queue.
			 */
			size_t get_message_count( void ) const;

			/**
			 * Returns true if there are messages in the queue.
			 * \return True if there are messages in the queue.
			 */
			bool has_messages( void ) const;

			/**
			 * Removes and returns the oldest message in the queue.  The message is subsequently deleted from the queue.
			 * \return The oldest message in the queue.
			 */
			MESSAGE_PTR pop_first( void ) throw( exception );

			/**
			 * Returns a message at the specified index in the queue.  The message is NOT deleted.  This behaviour is different than pop_first(void).
			 * \return The message at the specified index.
			 */
			MESSAGE_PTR get_message( unsigned int _idx ) throw( exception );

			/**
			 * Internal message queue instance.
			 */
			MESSAGE_VECTOR messages;

			string to_string( void ) const;

		protected:

			/**
			 * Number of message that the queue can hold.  Specified at instantiation time and can not be altered after that.
			 */
			unsigned int size;
		} ;
	}

	/**
	 * Class that manages protocol communications between server and client.
	 * A client process will have one instance of this class for every server that it is connected to.
	 * A server process will have one instance for every connected client.
	 */
	class MESSAGE_PROCESSOR
	{
	public:
		/**
		 * Constructor
		 */
		MESSAGE_PROCESSOR();

		/**
		 * Destructor.
		 */
		~MESSAGE_PROCESSOR();

		/**
		 * Parses a buffer and attempts to construct a message instance.
		 * \param _buffer Buffer containing the text representation of the message.
		 * \return Valid message instance if the buffer was parsed successfully.
		 */
		MESSAGE_PTR parse_message( const std::string& _buffer ) throw( exception );

		/**
		 * Sends a message to the remote endpoint.
		 * \param _msg Message to send.
		 * \param _fd File descriptor of the socket to which to write the message.
		 */
		void send_message( MESSAGE_PTR& _msg, int _fd ) throw( exception );


		/**
		 * Creates a message of type HELLO
		 * \return Valid message instance.
		 */
		MESSAGE_PTR create_hello_message( void );

		/**
		 * Creates a message of type PING
		 * \return Valid message instance.
		 */
		MESSAGE_PTR create_ping_message( void );

		/**
		 * Creates a message of type PONG
		 * \return Valid message instance.
		 */
		MESSAGE_PTR create_pong_message( void );

		/**
		 * Creates a message of type GET_MESSAGE
		 * \return Valid message instance.
		 */
		MESSAGE_PTR create_get_labels_message_request( ENUM_CONFIG_TYPES _type ) throw( exception );

		/**
		 * Creates a message of type GET_MESSAGE
		 * \return Valid message instance.
		 */
		MESSAGE_PTR create_get_labels_message_response( ENUM_CONFIG_TYPES _type ) throw( exception );

		/**
		 * Creates a message of type READ_STATUS_RAW_ANALOG
		 * \return Valid message instance.
		 */
		MESSAGE_PTR create_get_raw_adc_values( const std::string& _board_tag ) throw( exception );

		/**
		 * Creates a message of type READ_STATUS
		 * \return Valid message instance.
		 */
		MESSAGE_PTR create_get_status( const std::string& _board_tag ) throw( exception );

		/**
		 * Creates a message of type SET_PMIC_STATUS
		 * \param _val Bits of the status.  Both PMICs are modified using one byte.
		 * \return Valid message instance.
		 */
		MESSAGE_PTR create_set_pmic_status( const std::string& _board_tag, uint8_t _val ) throw( exception );

		/**
		 * Creates a message of type SET_STATUS
		 * \param _val Bits of the digital outputs.  All outputs are modified using on byte.
		 * \return Valid message instance.
		 */
		MESSAGE_PTR create_set_status( const std::string& _board_tag, uint8_t _val ) throw( exception );

		MESSAGE_PTR create_get_l1_cal_vals( const std::string& _board_tag ) throw( exception );
		MESSAGE_PTR create_get_l2_cal_vals( const std::string& _board_tag ) throw( exception );

		MESSAGE_PTR create_set_l1_cal_vals( const std::string& _board_tag, const CAL_VALUE_ARRAY& _vals ) throw( exception );
		MESSAGE_PTR create_set_l2_cal_vals( const std::string& _board_tag, const CAL_VALUE_ARRAY& _vals ) throw( exception );

		MESSAGE_PTR create_get_boot_count( const std::string& _board_tag ) throw( exception );

		MESSAGE_PTR create_error( int _code, const std::string& _message ) throw( exception );

		/**
		 * Processes an incoming message of type HELLO
		 */
		void process_hello_message( void ) throw( exception );

		/**
		 * Gets the latest incoming message of type PONG.  If such a message does not exist a nullptr is returned.
		 * \note Just because a nullptr is returned does not mean that a message was never received.  It could mean that the message was flushed out of the queue by other incomming messages before this method was called.
		 * \return Latest incomming PONG message.
		 */
		MESSAGE_PTR get_latest_incomming_pong( void );

		/**
		 * Gets the last outgoing message of type PONG.  If such a message does not exist a nullptr is returned.
		 * \return Last outgoing PING message.
		 * \see get_latest_incomming_pong
		 */
		MESSAGE_PTR get_latest_outgoing_ping( void );

		MESSAGE_PTR get_latest_incomming_of_type( ENUM_MESSAGE_TYPE _type );

		inline bool is_protocol_negotiated( void ) const {
			return this->protocol_negotiated;
		}

		string to_string( void ) const;

		/**
		 * Maximum protocol version that this client library supports.
		 */
		static unsigned int MAX_SUPPORTED_PROTOCOL;


	protected:

	private:
		/**
		 * Incomming message queue.
		 */
		MSG_PROC::MESSAGE_QUEUE* incomming_message_queue;

		/**
		 * Outgoing message queue.
		 */
		MSG_PROC::MESSAGE_QUEUE* outgoing_message_queue;

		/**
		 * Has the protocol been negotiated (HELLO received and accepted).  True if that is so.
		 */
		bool protocol_negotiated;

		/**
		 * Hidden copy constructor
		 */
		MESSAGE_PROCESSOR( const MESSAGE_PROCESSOR& _src );

		LOGGING::LOGGER __logger__;

	} ;
}

#endif /* SRC_INCLUDE_LIB_MESSAGE_PROCESSOR_HPP_ */
