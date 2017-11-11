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


#include "lib/socket_reader.hpp"
#include "lib/config.hpp"

#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

using namespace BBB_HVAC;

SOCKET_READER::SOCKET_READER()
{
	this->read_buffer = (char*)malloc(GC_BUFFER_SIZE);
	memset(this->read_buffer, 0, GC_BUFFER_SIZE);
	return;
}
SOCKET_READER::~SOCKET_READER()
{
	memset(this->read_buffer, 0, GC_BUFFER_SIZE);
	free(this->read_buffer);
	this->read_buffer = nullptr;
}

size_t SOCKET_READER::read(int _fd) throw(exception)
{
	ssize_t rc = recv(_fd, this->read_buffer, GC_BUFFER_SIZE, MSG_DONTWAIT);

	if(rc == -1)
	{
		throw(EXCEPTIONS::CONNECTION_ERROR(create_perror_string("Failed to read from client:")));
	}

	if(rc == 0)
	{
		/*
		 * Client has disconnected
		 */
		throw(EXCEPTIONS::CONNECTION_ERROR("Client connection closed."));
	}

	size_t start = 0;
	size_t nl_idx = 0;
	size_t last_nl = 0;

	while((nl_idx = this->find_nl_in_buffer(start,rc)) != (size_t)-1)
	{
		this->line_vector.push_back(string(this->read_buffer,start,(nl_idx - start) + 1));
		last_nl = nl_idx;
		start = nl_idx + 1;
	}

	if(last_nl < (size_t)(rc-1))
	{
		cerr << "Failed to find last NL in read buffer (" << last_nl << "|" << rc << ")" << endl;
	}

	return this->line_vector.size();
}

size_t SOCKET_READER::get_line_count(void) const
{
	return this->line_vector.size();
}
string SOCKET_READER::pop_first_line(void)
{
	if(this->get_line_count() < 1)
	{
		throw(runtime_error("Line vector is empty."));
	}

	string ret = this->line_vector[0];
	this->line_vector.erase(this->line_vector.begin());
	return ret;
}

size_t SOCKET_READER::find_nl_in_buffer(size_t _start,size_t _end)
{
	for(size_t i=_start; i<_end; i++)
	{
		if(read_buffer[i] == '\n')
		{
			return i;
		}
	}

	return -1;
}

