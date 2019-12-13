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


#include "lib/context.hpp"

#include "lib/context.hpp"
#include "lib/config.hpp"
#include "lib/message_processor.hpp"
#include "lib/socket_reader.hpp"
#include "lib/logger.hpp"
#include "lib/string_lib.hpp"
#include "lib/globals.hpp"
#include "lib/threads/logic_thread.hpp"
#include "lib/hvac_types.hpp"
#include "lib/threads/serial_io_thread.hpp"
#include "lib/threads/watchdog_thread.hpp"


#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <sstream>
#include <iostream>

#include <pthread.h>

using namespace BBB_HVAC;
using namespace BBB_HVAC::CLIENT;
using namespace BBB_HVAC::SERVER;
using namespace BBB_HVAC::EXCEPTIONS;

CLIENT_CONTEXT::CLIENT_CONTEXT( SOCKET_TYPE _st, const string& _path, uint16_t _port ) :
	BASE_CONTEXT( "CLIENT_CONTEXT", _st, _path, _port )
{
	INIT_LOGGER( "BBB_HVAC::CLIENT_CONTEXT" );
	this->is_in_client_mode = true;
	pthread_cond_init( & ( this->incomming_message_cond ), NULL );
}

ENUM_MESSAGE_CALLBACK_RESULT CLIENT_CONTEXT::process_message( ENUM_MESSAGE_DIRECTION _direction, BASE_CONTEXT* _ctx, const MESSAGE_PTR& _message ) throw( exception )
{
	/*
	 * This method will only be called by comm_thread.  At time of calling the mutex will have been obtained by the thread.
	 */
	ENUM_MESSAGE_CALLBACK_RESULT ret = ENUM_MESSAGE_CALLBACK_RESULT::IGNORED;

	if ( ( ret = BASE_CONTEXT::process_message( _direction, _ctx, _message ) ) != ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED )
	{
		/*
		 * So here's how this goes.
		 *
		 * The HMI_SHIM invokes the send_message_and_wait method of this instance.  This method (process_message) gets invoked from the comm thread (comm_thread).
		 * The HMI_SHIM lives in and invokes the send_message_and_wait method from whatever thread it is in.  We use the conditional variable below to awake
		 * the thread that the HMI_SHIM is waiting in for the message to arrive.
		 *
		 * This applies only to messages not processed and accepted by the BASE_CONTEXT.
		 *
		 */
		pthread_cond_signal( & ( this->incomming_message_cond ) );
		ret = ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
	}

	return ret;
}

CLIENT_CONTEXT::~CLIENT_CONTEXT()
{
	LOG_DEBUG( "Destroying CLIENT_CONTEXT" );
	this->abort_thread = true;
	pthread_cond_destroy( & ( this->incomming_message_cond ) );
}

void CLIENT_CONTEXT::connect( void ) throw( BBB_HVAC::EXCEPTIONS::CONNECTION_ERROR )
{
	switch ( this->st )
	{
		case SOCKET_TYPE::NONE:
			throw EXCEPTIONS::NETWORK_ERROR( "Invalid socket type (NONE) specified." );
			break;

		case SOCKET_TYPE::DOMAIN:
			if ( ::connect( this->remote_socket, ( sockaddr* )( &this->socket_struct_domain ), sizeof( struct sockaddr_un ) ) == -1 )
			{
				throw CONNECTION_ERROR( create_perror_string( "Failed to connect to server(domain) [" + string( this->socket_struct_domain.sun_path ) + "] : " ) );
			}

			break;

		case SOCKET_TYPE::TCPIP:
			if ( ::connect( this->remote_socket, ( sockaddr* )( &this->socket_struct_inet ), sizeof( struct sockaddr_in ) ) == -1 )
			{
				throw CONNECTION_ERROR( create_perror_string( "Failed to connect to server(TCP/IP): " ) );
			}

			break;
	}

	this->start_thread();
	//pthread_create(&this->thread_ctx, nullptr, (void* (*)(void*))comm_thread_func, this);
	timespec t;
	memset( &t, 0, sizeof( struct timespec ) );

	while ( this->message_processor->is_protocol_negotiated() == false )
	{
		t.tv_nsec = GC_NSEC_TIMEOUT;
		this->nsleep( &t );
	}

	LOG_DEBUG( "Connected to remote LOGIC_CORE" );
	return;
}

bool CLIENT_CONTEXT::send_message( MESSAGE_PTR& _message ) throw( exception )
{
	this->obtain_lock();

	try
	{
		this->message_processor->send_message( _message, this->remote_socket );
	}
	catch ( const exception& _e )
	{
		this->release_lock();
		throw runtime_error( string( "Failed to send message: " ) + _e.what() );
	}

	this->release_lock();
	return true;
}

MESSAGE_PTR CLIENT_CONTEXT::send_message_and_wait( MESSAGE_PTR& _message ) throw( exception )
{
	MESSAGE_PTR ret;
	ENUM_MESSAGE_TYPE msg_type = _message->get_message_type()->type;
	this->obtain_lock();

	try
	{
		this->message_processor->send_message( _message, this->remote_socket );
	}
	catch ( const exception& _e )
	{
		this->release_lock();
		throw runtime_error( string( "Failed to send message: " ) + _e.what() );
	}

	timespec timeout_time;
	memset( &timeout_time, 0, sizeof( struct timespec ) );
	/*
	 * We wait for two seconds for the reply to the message to get back to us before aborting the thread and bringing the whole application down.
	 * XXX - need to figure out a better, less brutal way of dealing with error conditions in wait states.
	 */
	timeout_time.tv_sec = time( nullptr ) + 2;
	int rc = pthread_cond_timedwait( & ( this->incomming_message_cond ), & ( this->mutex ), &timeout_time );

	if ( rc != 0 )
	{
		this->abort_thread = true;

		if ( rc == ETIMEDOUT )
		{
			LOG_ERROR( "Timed out waiting on a conditional.  Suspect something hinky.  Aborting." );
			THROW_EXCEPTION( runtime_error, "Time out on a conditional." );
		}
		else
		{
			LOG_ERROR( "Failed to wait on a conditional: " + num_to_str( rc ) );
			THROW_EXCEPTION( runtime_error, "Failed to wait on a conditional: " + num_to_str( rc ) );
		}
	}

	ret = this->message_processor->get_latest_incomming_of_type( msg_type );
	this->release_lock();
	return ret;
}

void CLIENT_CONTEXT::disconnect( void ) throw( EXCEPTIONS::PROTOCOL_ERROR )
{
	this->abort_thread = true;
	pthread_join( this->thread_ctx, nullptr );
	return;
}
