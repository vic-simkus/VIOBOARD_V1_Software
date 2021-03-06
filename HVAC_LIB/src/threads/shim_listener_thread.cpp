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

#include "lib/threads/shim_listener_thread.hpp"
#include "lib/context.hpp"
#include "lib/globals.hpp"

#include <memory>
using namespace std;

#include <sys/socket.h>

#include <poll.h>

namespace BBB_HVAC
{
	using namespace SERVER;

	SHIM_LISTENER::SHIM_LISTENER( SOCKET_TYPE _st, const string& _path, uint16_t _port ) : THREAD_BASE( "SHIM_LISTENER" )
	{
		INIT_LOGGER( "BBB_HVAC::SHIM_LISTENER" ) ;
		this->server_ctx = nullptr;

		this->socket_type = _st;
		this->path = _path;
		this->port  = _port;
	}

	SHIM_LISTENER::~SHIM_LISTENER()
	{
		delete this->server_ctx;
	}

	void SHIM_LISTENER::init( void )
	{
		this->server_ctx = new HS_SERVER_CONTEXT( this->socket_type, this->path, this->port );
		int reuse = 1;

		switch ( this->server_ctx->st )
		{
			case SOCKET_TYPE::NONE:
				throw EXCEPTIONS::NETWORK_ERROR( "Invalid socket type (NONE) specified." );
				break;

			case SOCKET_TYPE::DOMAIN:
				if ( ::bind( this->server_ctx->remote_socket, ( const struct sockaddr* ) & ( this->server_ctx->socket_struct_domain ), sizeof( struct sockaddr_un ) ) == -1 )
				{
					throw EXCEPTIONS::NETWORK_ERROR( create_perror_string( "Failed to bind to domain socket" ) );
				}

				break;

			case SOCKET_TYPE::TCPIP:

				if ( setsockopt( this->server_ctx->remote_socket, SOL_SOCKET, SO_REUSEPORT, ( const int* )&reuse, sizeof( reuse ) )  < 0 )
				{
					throw EXCEPTIONS::NETWORK_ERROR( create_perror_string( "Failed to set TPC/IP socket option (SO_REUSEPORT) on FD [" + num_to_str( this->server_ctx->remote_socket ) + "]" ) );
				}

				if ( setsockopt( this->server_ctx->remote_socket, SOL_SOCKET, SO_REUSEADDR, ( const int* )&reuse, sizeof( reuse ) )  < 0 )
				{
					throw EXCEPTIONS::NETWORK_ERROR( create_perror_string( "Failed to set TPC/IP socket options (SO_REUSEADDR) on FD [" + num_to_str( this->server_ctx->remote_socket ) + "]" ) );
				}

				if ( ::bind( this->server_ctx->remote_socket, ( const struct sockaddr* ) & ( this->server_ctx->socket_struct_inet ), sizeof( struct sockaddr_in ) ) == -1 )
				{
					throw EXCEPTIONS::NETWORK_ERROR( create_perror_string( "Failed to bind to TCP/IP socket" ) );
				}

				break;
		}

		return;
	}

	bool SHIM_LISTENER::thread_func( void )
	{
		LOG_INFO( "Starting shim listening thread." );

		if ( listen( this->server_ctx->remote_socket, 10 ) == -1 )
		{
			LOG_ERROR( create_perror_string( "Failed to listen on socket:" ) );
			exit( -1 );
		}

		LOG_DEBUG( "Listening on socket." );
		HS_CLIENT_CONTEXT* client_ctx;
		int client_fd = 0;
		struct sockaddr_un client_addr;
		socklen_t client_addr_len = sizeof( struct sockaddr_un );
		struct pollfd fds;

		while ( this->abort_thread == false )
		{
			fds.fd = this->server_ctx->remote_socket;
			fds.events = POLLIN;
			int fds_ready_num = poll( &fds, 1, 100 );

			if ( fds_ready_num == 0 )
			{
				continue;
			}

			memset( &client_addr, 0, client_addr_len );
			client_fd = accept( this->server_ctx->remote_socket, ( sockaddr* ) & client_addr, &client_addr_len );

			if ( client_fd == -1 )
			{
				LOG_ERROR( create_perror_string( "accept() failed" ) );
				GLOBALS::global_exit_flag = true;
				break;
			}
			else
			{
				client_ctx = new HS_CLIENT_CONTEXT( client_fd );
				client_ctx->start_thread();
			}
		}

		return true;
	}

}
