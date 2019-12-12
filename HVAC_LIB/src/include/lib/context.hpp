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


#ifndef SRC_LIB_CONTEXT_HPP_
#define SRC_LIB_CONTEXT_HPP_

#include <sys/un.h>
#include <netinet/ip.h>
#include <pthread.h>

#include "lib/logger.hpp"
#include "lib/exceptions.hpp"
#include "lib/message_callbacks.hpp"
#include "lib/tprotect_base.hpp"
#include "lib/hvac_types.hpp"
#include "lib/threads/thread_base.hpp"
#include "lib/config.hpp"

namespace BBB_HVAC
{
	class MESSAGE_PROCESSOR;

	enum class SOCKET_TYPE : unsigned char
	{
		NONE = 0,
		DOMAIN,
		TCPIP
	};

	/**
	 * Base class for all contexts.
	 * Contains a socket and unix domain address struct.
	 */
	class BASE_CONTEXT : public MESSAGE_CALLBACK_BASE, public THREAD_BASE
	{
		public:
			DEF_LOGGER;

			friend void BBB_HVAC::comm_thread_func( void* );

			/**
			 * Invoked by the comm_thread every time a new message comes in.  The base implementation handles the HELLO, PING, and PONG messages.  It ignores all other
			 * types of messages.  The subclasses are expected to call the method in the base class and handle any messages that are not ignored any way they see fit.
			 */
			ENUM_MESSAGE_CALLBACK_RESULT process_message( ENUM_MESSAGE_DIRECTION _direction, BASE_CONTEXT* _ctx, const MESSAGE_PTR& _message ) throw( exception );

			/**
			 * Constructor
			 */
			BASE_CONTEXT( const string& _tag, SOCKET_TYPE _st, const string& _path, uint16_t _port );

			/**
			 * Destructor
			 */
			virtual ~BASE_CONTEXT();

			/**
			 * File descriptor
			 */
			int remote_socket;

			/**
			 * Address struct.
			 */
			struct sockaddr_un socket_struct_domain;
			struct sockaddr_in socket_struct_inet;

			/**
			 * Client thread context.
			 */
			pthread_t thread_ctx;

			/**
			 * Associated message processor.
			 */
			MESSAGE_PROCESSOR* message_processor;

			/**
			 * Human readable instance tag intended for logging/debugging.
			 */
			string instance_tag;

			struct timeval select_timeout_tv;

			int timeout_counter;

			const unsigned int max_pp_timeout;

			timespec curr_time;
			SOCKET_TYPE st;

		protected:
			bool select_timeout_happened( void ) throw( exception );
			bool send_initial_ping( void ) throw( exception );

			bool thread_func( void );

			LOGGING::LOGGER* logger;



			bool is_in_client_mode;

	};

	/**
	 * Server namespace.  Everything in here will be used only by the server process.
	 */
	namespace SERVER
	{
		/**
		 * Server context.  This will be used by the server.  This context doesn't do much other than contain the thread context, quit flag, and listening socket.
		 */
		class HS_SERVER_CONTEXT: public BASE_CONTEXT
		{
			public:
				DEF_LOGGER;
				/**
				 * Constructor.
				 */
				HS_SERVER_CONTEXT( SOCKET_TYPE _st, const string& _path, uint16_t _port );

				/**
				 * Destructor.
				 */
				~HS_SERVER_CONTEXT();
		};

		/**
		 * Server client context.  This will be used by the server process.  Every connected shim has an instance of this class.  Basically all message processing is done here.
		 */
		class HS_CLIENT_CONTEXT: public BASE_CONTEXT
		{
			public:
				DEF_LOGGER;
				/**
				 * Constructor
				 * \param _client_socket Client file descriptor returned by the 'accept' call.
				 */
				HS_CLIENT_CONTEXT( int _client_socket );

				/**
				 * Destructor.
				 */
				~HS_CLIENT_CONTEXT();

				/**
				 * Processes all incoming messages.  Message is first offered to BASE_CONTEXT implementation of the method.  If that method ignores it then this method will handle the message.
				 */
				virtual ENUM_MESSAGE_CALLBACK_RESULT process_message( ENUM_MESSAGE_DIRECTION _direction, BASE_CONTEXT* _ctx, const MESSAGE_PTR& _message ) throw( exception );

		};
	}

	/**
	 * Client namespace.  Everything here will be used by the shim process.
	 */
	namespace CLIENT
	{
		class CLIENT_CONTEXT: public BASE_CONTEXT
		{
			public:
				/**
				 * Logger.
				 */
				DEF_LOGGER;

				/**
				 * Constructor.
				 */
				~CLIENT_CONTEXT();

				/**
				 * Connets to the server process.
				 */
				void connect( void ) throw( BBB_HVAC::EXCEPTIONS::CONNECTION_ERROR );

				/**
				 * Disconnects from the server process.
				 */
				void disconnect( void ) throw( EXCEPTIONS::PROTOCOL_ERROR );

				/**
				 * Sends a message to the remote peer and waits for a response.
				 * \return An instance of the reply message
				 */
				MESSAGE_PTR send_message_and_wait( MESSAGE_PTR& _message ) throw( exception );

				bool send_message( MESSAGE_PTR& _message ) throw( exception );


				ENUM_MESSAGE_CALLBACK_RESULT process_message( ENUM_MESSAGE_DIRECTION _direction, BASE_CONTEXT* _ctx, const MESSAGE_PTR& _message ) throw( exception );

				inline static CLIENT_CONTEXT* create_instance( SOCKET_TYPE _st, const string& _path, uint16_t _port ) {
					return new CLIENT_CONTEXT( _st, _path, _port );
				}

			protected:

				/**
				 * Constructor.
				 * We hide the constructor so that we can't just create instances on the stack because the comm_thread takes ownership of the pointer.
				 */
				CLIENT_CONTEXT( SOCKET_TYPE _st, const string& _path, uint16_t _port );

				pthread_cond_t incomming_message_cond;

			private:

		};
	}
}
#endif /* SRC_LIB_CONTEXT_HPP_ */
