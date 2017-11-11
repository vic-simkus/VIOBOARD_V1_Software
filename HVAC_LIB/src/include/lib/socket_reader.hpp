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


#ifndef SRC_INCLUDE_LIB_SOCKET_READER_HPP_
#define SRC_INCLUDE_LIB_SOCKET_READER_HPP_

#include "exceptions.hpp"

#include <vector>

namespace BBB_HVAC
{
	using namespace std;

	/**
	 * Reads data available in the supplied socket and turns it into lines delimited by a newline (\\n) character.
	 */
	class SOCKET_READER
	{
	public:
		/**
		 * Constructor
		 */
		SOCKET_READER();

		/**
		 * Destructor
		 */
		~SOCKET_READER();

		/**
		 * Reads all available data from the supplied file descriptor.
		 * \param _fd File descriptor to read from.
		 * \return Number of lines available for processing.  Note:  This does not mean how many lines were read during this method invocation.  The returned number of lines may include number of lines stored in the internal buffer that have not yet been consumed.
		 */
		size_t read(int _fd) throw(exception);

		/**
		 * Gets the number of lines in the internal buffer that are available for processing.
		 * \return Number of lines in the internal buffer.
		 */
		size_t get_line_count(void) const;

		/**
		 * Returns the first (oldest) line from the internal buffer.  The line is then deleted from the buffer.
		 * \return Line to be processed.
		 */
		string pop_first_line(void);

	protected:
		/**
		 * Temporary read buffer.  Size determined by GC_BUFFER_SIZE.
		 */
		char* read_buffer;

		/**
		 * Finds the next occurrence of a new line character in the read_buffer.
		 */
		size_t find_nl_in_buffer(size_t _start, size_t _end);

		/**
		 * Line buffer.  Lines read, but not yet processed.
		 */
		vector<string> line_vector;

	private:
	};
}

#endif /* SRC_INCLUDE_LIB_SOCKET_READER_HPP_ */
