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

	SHIM_LISTENER::SHIM_LISTENER() : THREAD_BASE("SHIM_LISTENER")
	{
		this->logger = new LOGGING::LOGGER();
		INIT_LOGGER_P("BBB_HVAC::SHIM_LISTENER");
		this->server_ctx = nullptr;
	}

	SHIM_LISTENER::~SHIM_LISTENER()
	{
		delete this->server_ctx;
		delete this->logger;
		this->logger = nullptr;
	}

	void SHIM_LISTENER::init(void) throw(exception)
	{
		this->server_ctx = new HS_SERVER_CONTEXT();

		if(bind(this->server_ctx->remote_socket, (const struct sockaddr*) &(this->server_ctx->socket_struct), sizeof(struct sockaddr_un)) == -1)
		{
			throw EXCEPTIONS::NETWORK_ERROR(create_perror_string("Failed to bind to domain socket."));
		}

		return;
	}

	bool SHIM_LISTENER::thread_func(void)
	{
		LOG_INFO_P("Starting shim listening thread.");

		if(listen(this->server_ctx->remote_socket, 10) == -1)
		{
			LOG_ERROR_P(create_perror_string("Failed to listen on socket:"));
			exit(-1);
		}

		LOG_DEBUG_P("Listening on socket.");
		HS_CLIENT_CONTEXT* client_ctx;
		int client_fd = 0;
		struct sockaddr_un client_addr;
		socklen_t client_addr_len = sizeof(struct sockaddr_un);
		struct pollfd fds;

		while(this->abort_thread == false)
		{
			fds.fd = this->server_ctx->remote_socket;
			fds.events = POLLIN;
			int fds_ready_num = poll(&fds, 1, 100);

			if(fds_ready_num == 0)
			{
				continue;
			}

			memset(&client_addr, 0, client_addr_len);
			client_fd = accept(this->server_ctx->remote_socket, (sockaddr*) & client_addr, &client_addr_len);

			if(client_fd == -1)
			{
				LOG_ERROR_P(create_perror_string("accept() failed"));
				GLOBALS::global_exit_flag = true;
				break;
			}
			else
			{
				client_ctx = new HS_CLIENT_CONTEXT(client_fd);
				client_ctx->start_thread();
			}
		}

		return true;
	}

}