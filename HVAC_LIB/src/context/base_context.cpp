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

bool BASE_CONTEXT::send_initial_ping(void) throw(exception)
{
	/*
	 * We have never sent a PING.  Send out a PING to the remote
	 */
	//LOG_DEBUG( "Sending initial PING" );
	bool rc = false;
	MESSAGE_PTR m = this->message_processor->create_ping_message();

	try
	{
		this->message_processor->send_message(m, this->remote_socket);
	}
	catch(EXCEPTIONS::MESSAGE_ERROR& e)
	{
		LOG_ERROR(string("Failed to send ping to remote: ") + e.what());
		rc = true;
	}

	this->timeout_counter = 0;
	return rc;
}

bool BASE_CONTEXT::select_timeout_happened(void) throw(exception)
{
	/*
	 * Gets invoked whenever a select timeout happens in the comm thread.  Here we decide if we're gonna drop the client connection or not.
	 */
	bool rc = false;

	if(this->timeout_counter == -1)
	{
		/*
		 * initial ping has not been sent.
		 */
		return this->send_initial_ping();
	}

	this->timeout_counter += 1;

	if(this->timeout_counter >= (GC_CLIENT_PING_DIVIDER - 1))
	{
		/*
		 * First check if a PONG has been received within last GC_CLIENT_THREAD_SELECT_TIME * GC_CLIENT_PING_DIVIDER.
		 * If we have not received a PONG drop the connection as the remote must have it's drawers in a twist.
		 */
		/*
		 * XXX - We have an intermittent crash here on client disconnect.
		 */
		MESSAGE_PTR o_ping = this->message_processor->get_latest_outgoing_ping();
		MESSAGE_PTR i_pong = this->message_processor->get_latest_incomming_pong();

		if(i_pong.get() == nullptr)
		{
			/*
			 * We have not received a PONG response ever  That includes the initial ping.
			 */
			if((curr_time.tv_sec - o_ping->get_message_sent_timestamp()->tv_sec) >= this->max_pp_timeout)
			{
				/*
				 * If the alloted time for the initial PONG response has elapsed, drop the connection.
				 */
				LOG_ERROR("Dropping connection; failed to get a PONG response from remote in the last " + num_to_str(this->max_pp_timeout) + " seconds.");
				LOG_DEBUG("timeout_counter=" + num_to_str(timeout_counter));
				rc = true;
			}
		}
		else
		{
			/*
			 * We have received a PONG recently
			 */
			const timespec* ping_t = o_ping->get_message_sent_timestamp();
			const timespec* pong_t = i_pong->get_message_received_timestamp();
			time_t elapsed_time = (pong_t->tv_sec - ping_t->tv_sec);

			if(elapsed_time > this->max_pp_timeout)
			{
				LOG_ERROR("Dropping connection; last PONG too old: " + num_to_str((i_pong->get_message_received_timestamp()->tv_sec - o_ping->get_message_sent_timestamp()->tv_sec)) + " seconds.");
				rc = true;
			}
			else
			{
				MESSAGE_PTR m = this->message_processor->create_ping_message();

				try
				{
					this->message_processor->send_message(m, this->remote_socket);
				}
				catch(EXCEPTIONS::MESSAGE_ERROR& e)
				{
					LOG_ERROR(string("Failed to send ping to remote: ") + e.what());
					rc = true;
				}
			}
		}
	} //Not the first time through the loop and also timeout_counter != divider

	return rc;
}

BASE_CONTEXT::BASE_CONTEXT(const string& _tag) :
	THREAD_BASE(_tag), max_pp_timeout((GC_CLIENT_THREAD_SELECT_TIME * GC_CLIENT_PING_DIVIDER))
{
	this->logger = new LOGGING::LOGGER("BBB_HVAC::BASE_CONTEXT[" + _tag + "]");

	if((this->remote_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
	{
		throw CONNECTION_ERROR(create_perror_string("Failed to create connection socket"));
	}

	this->socket_struct.sun_family = AF_UNIX;
	strncpy(this->socket_struct.sun_path, GC_LOCAL_COMMAND_SOCKET, sizeof(this->socket_struct.sun_path));
	this->thread_ctx = 0;
	this->abort_thread = false;
	this->instance_tag = _tag;
	this->timeout_counter = -1;
	this->message_processor = new MESSAGE_PROCESSOR();
	this->is_in_client_mode = false;
	return;
}

BASE_CONTEXT::~BASE_CONTEXT()
{
	close(this->remote_socket);
	this->remote_socket = -1;
	memset(&this->socket_struct, 0, sizeof(struct sockaddr_un));
	this->thread_ctx = 0;
	delete this->message_processor;
	this->message_processor = 0;
	this->timeout_counter = 0;
	memset(&this->curr_time, 0, sizeof(struct timespec));
	memset(&this->select_timeout_tv, 0, sizeof(struct timeval));
	delete this->logger;
	this->logger = nullptr;
	return;
}

ENUM_MESSAGE_CALLBACK_RESULT BASE_CONTEXT::process_message(ENUM_MESSAGE_DIRECTION, BASE_CONTEXT*, const MESSAGE_PTR& _message) throw(exception)
{
	if(_message->get_message_type()->type == ENUM_MESSAGE_TYPE::PING)
	{
		MESSAGE_PTR m = this->message_processor->create_pong_message();
		this->message_processor->send_message(m, this->remote_socket);
		return ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
	}
	else if(_message->get_message_type()->type == ENUM_MESSAGE_TYPE::HELLO)
	{
		this->message_processor->process_hello_message();
		return ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
	}
	else if(_message->get_message_type()->type == ENUM_MESSAGE_TYPE::PONG)
	{
		// ignore
		return ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
	}
	else
	{
		return ENUM_MESSAGE_CALLBACK_RESULT::IGNORED;
	}
}

bool BASE_CONTEXT::thread_func(void)
{
	fd_set read_fds;
	int rc = 0;

	try
	{
		MESSAGE_PTR m;
		m = this->message_processor->create_hello_message();
		this->message_processor->send_message(m, this->remote_socket);
	}
	catch(const exception& e)
	{
		throw EXCEPTIONS::CONNECTION_ERROR(string("Failed to establish connection: ") + e.what());
	}

	SOCKET_READER socket_reader;

	/*
	 * Need to convert to a centralized message processor starting here.
	 */

	try
	{
		const int usec_timeout = (GC_CLIENT_THREAD_SELECT_TIME * 1000) / GC_CLIENT_PING_DIVIDER;

		while(this->abort_thread == false)
		{
			memset(&(this->curr_time), 0, sizeof(struct timespec));

			if(clock_gettime(CLOCK_MONOTONIC, &(this->curr_time)) != 0)
			{
				LOG_ERROR_P(create_perror_string("Failed to get current time"));
				this->abort_thread = true;
				continue;
			}

			FD_ZERO(&read_fds);
			FD_SET(this->remote_socket, &read_fds);
			memset(&(this->select_timeout_tv), 0, sizeof(struct timeval));
			this->select_timeout_tv.tv_sec = GC_CLIENT_THREAD_SELECT_TIME;
			this->select_timeout_tv.tv_usec = usec_timeout;
			rc = select(this->remote_socket + 1, &read_fds, nullptr, nullptr, &(this->select_timeout_tv));
			this->obtain_lock();

			if(rc == -1)
			{
				/*
				 * If select returns an error, log it and exit thread.
				 */
				LOG_ERROR_P(create_perror_string("Select on remote socket failed"));
				this->abort_thread = true;
				this->release_lock();
				continue;
			}
			else if(rc == 0)
			{
				/*
				 * Select timed out without any new data.  Timeout period set by GC_CLIENT_THREAD_SELECT_TIME
				 *timeout_counter == -1 during first entry into the loop
				 */
				if(this->is_in_client_mode == false)
				{
					/*
					 * Only do this in the server mode of the thread.
					 */
					if((this->abort_thread = this->select_timeout_happened()) == true)
					{
						/*
						 * Connection timed out.
						 */
						this->release_lock();
						continue;
					}
				}
			} // rc = 0; timeout
			else if(rc == 1)
			{
				/*
				 * We do have data available for reading
				 */

				/*
				 * Reset the timeout counter if this is not the first time through the loop.
				 * If it is the first time through the loop we do want to send the initial ping.
				 */
				if(this->timeout_counter != -1)
				{
					this->timeout_counter = 0;
				}

				try
				{
					size_t read_count = socket_reader.read(this->remote_socket);

					for(size_t i = 0; i < read_count; i++)
					{
						string s = socket_reader.pop_first_line();
						/*
						 * Process all of the read lines here.
						 */
						MESSAGE_PTR m;

						try
						{
							m = this->message_processor->parse_message(s);
						}
						catch(exception& e)
						{
							/*
							 * Log and ignore a bad message.
							 */
							LOG_DEBUG_P("Failed to parse message: " + string(e.what()));
							continue; //continue processing in the 'for' loop
						}

						try
						{
							this->process_message(ENUM_MESSAGE_DIRECTION::IN, this, m);
						}
						catch(const exception& e)
						{
							/*
							This is where we end up if the client really screws up.
							For example if they try to read from an IO board that doesn't exist.  The exception bubbles up to us here and we just punt.
							*/
							LOG_DEBUG_P("Failed to process message:" + string(e.what()));
							LOG_DEBUG_P("Offending message: " + m->to_string());
							this->abort_thread = true;
							break;	// break out of the line processing loop
						}
						catch(...)
						{
							LOG_DEBUG_P("Unspecified exception caught while processing remote message");
							this->abort_thread = true;
							break;	// break out of the line processing loop
						}
					}
				}
				catch(exception& e)
				{
					LOG_DEBUG_P("General failure to process remote request: " + string(e.what()));
					this->abort_thread = true;
				}
				catch(...)
				{
					LOG_DEBUG_P("Unknown exception caught while processing remote request.");
					this->abort_thread = true;
				}
			} // Select indicated that a FD is ready to be read from.

			this->release_lock();
		} //main select loop
	}
	catch(exception& e)      // main "try" block.
	{
		LOG_DEBUG_P(string("Unhandled exception caught in the client thread: ") + e.what());
	}
	catch(...)
	{
		LOG_DEBUG_P("Unhandled exception of unknown type caught in the client thread.");
	}

	close(this->remote_socket);
	return true;
}
