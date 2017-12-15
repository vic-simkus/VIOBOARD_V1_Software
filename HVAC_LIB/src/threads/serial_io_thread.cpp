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


#include <iostream>
using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fstream>

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

#include "lib/threads/serial_io_thread.hpp"
#include "lib/string_lib.hpp"
#include "lib/config.hpp"
#include "lib/logger.hpp"
#include "lib/exceptions.hpp"

#include "lib/globals.hpp"
#include "lib/memory_management.hpp"

#include <poll.h>

using namespace BBB_HVAC;
using namespace BBB_HVAC::IOCOMM;

#define ASSEMBLE_16INT(_lsb,_msb)  ((uint16_t)((((uint16_t)_lsb)) | ((uint16_t)_msb<<8)))

/*
Byte 1 - binary marker
Byte 2 - Call index
Byte 3 - Response code LSB
Byte 4 - Response code MSB
Byte 5 - Length LSB
Byte 6 - Length MSB
*/
#define RESP_HEAD_SIZE 6

#define RESP_HEAD_BM_IDX		0
#define RESP_HEAD_CI_IDX		1
#define RESP_HEAD_STAT_LSB_IDX	2
#define RESP_HEAD_STAT_MSB_IDX	3
#define RESP_HEAD_LEN_LSB_IDX	4
#define RESP_HEAD_LEN_MSB_IDX	5

void SER_IO_COMM::serial_port_close(void)
{
	tcflush(this->serial_fd, TCIOFLUSH);
	tcflow(this->serial_fd, TCOOFF);
	tcsetattr(this->serial_fd, TCSANOW, &this->old_tio);
	close(this->serial_fd);
	this->serial_fd = 0;
	this->reset_buffer_context();
	return;
}

SER_IO_COMM::~SER_IO_COMM()
{
	if(this->serial_fd == 0)
	{
		return;
	}

	this->serial_port_close();
	unlink(this->lock_file.data());
	free(this->buffer);
	this->buffer = nullptr;
	delete this->outgoing_messages;
	this->outgoing_messages = nullptr;
	return;
}

SER_IO_COMM::SER_IO_COMM(const char* _tty, const string& _tag,bool _debug) :
	IO_COMM_BASE(_tag)
{
	this->logger->configure("BBB_HVAC::SERIAL_IO[" + _tag + "]");
	this->tty_dev = generate_port_device_file_name(_tty);
	this->lock_file = generate_lock_file_name(_tty);
	this->buffer = (unsigned char*) calloc(GC_SERIAL_BUFF_SIZE, sizeof(unsigned char));
	this->serial_fd = 0;
	this->active_table = &this->table_a;
	this->passive_table = &this->table_b;
	this->outgoing_messages = new OUTGOING_MESSAGE_QUEUE(this->tag + "/" + "OUT_QUEUE");
	this->reset_buffer_context();
	this->board_has_reset = false;
	this->in_debug_mode = _debug;
	memset(this->buffer,0xFF,GC_SERIAL_BUFF_SIZE);
	return;
}

void SER_IO_COMM::writer_thread_func(void)
{
	LOG_DEBUG_P("Write event loop thread starting.");

	if(!this->write_event_loop())
	{
		LOG_DEBUG_P("Thread's write event loop returned 'false'.");
	}

	LOG_DEBUG_P("Write event loop thread ending.");
	//pthread_exit( nullptr );
	return;
}

bool SER_IO_COMM::thread_func(void)
{
	pthread_t writer_thread_id ;
	pthread_create(&writer_thread_id, nullptr, (void* (*)(void*))serial_io_shim_func, this);
	/*
	 * This delay is needed.  Otherwise the writer thread doesn't get started before the main even loop thread
	 * and the board reset goes missing since the write thread is not there to wait for the conditional.
	 */
	timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 250000000;	// quarter second
	this->nsleep(&ts);
	LOG_DEBUG_P("Main event loop thread starting.");

	if(!this->main_event_loop())
	{
		LOG_DEBUG_P("Thread's main event loop returned 'false'.");
	}

	LOG_DEBUG_P("Main event loop thread ending.");
	pthread_join(writer_thread_id, nullptr);
	//pthread_exit( nullptr );
	return true;
}

string SER_IO_COMM::get_locking_pid(const string& _file)
{
	ifstream pid_file(_file);
	string line;
	std::getline(pid_file, line);
	pid_file.close();
	trim(line);
	return line;
}

string SER_IO_COMM::get_locking_pid_command(const string& _pid)
{
	ifstream pid_file("/proc/" + _pid + "/cmdline");
	string line;
	std::getline(pid_file, line);
	pid_file.close();
	trim(line);
	return line;
}

bool SER_IO_COMM::is_process_alive(const string& _pid)
{
	string proc_file = "/proc/" + _pid;

	if(access(proc_file.data(), F_OK) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

string SER_IO_COMM::generate_lock_file_name(const char* _tty)
{
	return string("/var/lock/LCK..") + _tty;
}

string SER_IO_COMM::generate_port_device_file_name(const char* _tty)
{
	return string("/dev/") + _tty;
}

bool SER_IO_COMM::try_lock_serial(void)
{
	bool ret = false;
	LOG_DEBUG_P("Checking lock file: " + this->lock_file);

	if(access(this->lock_file.data(), F_OK) == 0)
	{
		//lock file exists
		string pid = get_locking_pid(this->lock_file);

		if(is_process_alive(pid))
		{
			LOG_ERROR_P("Port is locked and locking process is alive: " + get_locking_pid_command(pid));
			ret = false;
		}
		else
		{
			LOG_DEBUG_P("Port is locked, but locking process is dead.");
			;
			unlink(this->lock_file.data());
			ret = true;
		}
	}
	else
	{
		ret = true;
	}

	if(ret)
	{
		/*
		 * Either lock file does not exist or we were able to clear it.
		 */
		ofstream lock_file(this->lock_file);

		if(!lock_file.is_open())
		{
			LOG_ERROR_P("Failed to open lock file for writting.");
			ret = false;
		}
		else
		{
			/*
			 * I don't know why we're preceding the PID with a tab.  Gonna guess something to do with minicom
			 */
			lock_file << "\t" << getpid() << endl;
			lock_file.close();
			ret = true;
		}
	}

	return ret;
}

bool SER_IO_COMM::drain_serial(void)
{
	bool rc = true;
	long int bytes_read = 0;

	while(bytes_read < 128)
	{
		/*
		 * If I make the type ssize_t arithmetic with size_t fails
		 */
		long int rc = read(this->serial_fd, buffer + this->buffer_context.buffer_idx, GC_SERIAL_BUFF_SIZE - this->buffer_context.buffer_idx);

		// DEBUG SET A.1.  Also enable DEBUG SET A.2 at the same time.
		//long int rc = read( this->serial_fd, buffer + this->buffer_context.buffer_idx, 1 );

		if(rc == 0)
		{
			/*
			 * Nothing more to read
			 */
			rc = true;
			break;
		}
		else if(rc < 0)
		{
			LOG_ERROR_P(create_perror_string("Read"));
			rc = false;
			break;
		}

		bytes_read += rc;

		// DEBUG SET A.2
		//cerr << "[" << this->buffer_context.buffer_idx << "] -- 0x" << char_to_hex( buffer[this->buffer_context.buffer_idx]) << " -- " <<  (buffer[this->buffer_context.buffer_idx] != 0x0A ? char(buffer[this->buffer_context.buffer_idx]) : (char)' ')   << endl;
		this->buffer_context.buffer_idx = this->buffer_context.buffer_idx + rc;

		if(this->buffer_context.buffer_idx >= GC_SERIAL_BUFF_SIZE)
		{
			/*
			 * Overflow
			 */
			LOG_ERROR_P("Serial receive buffer overflowed.  Data has been lost.");
			this->reset_buffer_context();
			memset(this->buffer,0x00,GC_SERIAL_BUFF_SIZE);
		}
	}

	return rc;
}

bool SER_IO_COMM::clear_to_send(void) const
{
	unsigned int flow_ctrl_status;
	unsigned int output_queue;

	if(ioctl(this->serial_fd, TIOCOUTQ, &output_queue) != 0)
	{
		LOG_ERROR_P(create_perror_string("Failed get IOCTL port status"));
		return false;
	}

	if(ioctl(this->serial_fd, TIOCMGET, &flow_ctrl_status) != 0)
	{
		LOG_ERROR_P(create_perror_string("Failed get IOCTL port status"));
		return false;
	}
	else
	{
		if(output_queue == 0 && (flow_ctrl_status & TIOCM_CTS))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}
ENUM_ERRORS SER_IO_COMM::serial_port_open(void)
{
	this->serial_fd = open(this->tty_dev.data(), O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);

	if(this->serial_fd < 0)
	{
		LOG_ERROR_P(create_perror_string("Serial port open"));
		return ENUM_ERRORS::ERR_PORT_OPEN;
	}

	memset(&this->old_tio, 0, sizeof(struct termios));
	memset(&this->current_tio, 0, sizeof(struct termios));
	/*
	 * Save existing parameters and read in existing parameters for editing.
	 */
	tcgetattr(this->serial_fd, &this->old_tio);
	tcgetattr(this->serial_fd, &this->current_tio);
	/*
	 * Set input and output speed
	 */
	cfsetispeed(&this->current_tio, GC_SERIAL_BAUD_RATE);
	cfsetospeed(&this->current_tio, GC_SERIAL_BAUD_RATE);
	/*
	 * Enable receiver and set local mode
	 */
	this->current_tio.c_cflag |= (CREAD | CLOCAL);
	/*
	 * Set for 8N1
	 */
	this->current_tio.c_cflag &= ~PARENB;
	this->current_tio.c_cflag &= ~CSTOPB;
	this->current_tio.c_cflag &= ~CSIZE;
	this->current_tio.c_cflag |= CS8;
	/*
	 * Enable raw mode; disable canonical mode, echo, and signals
	 */
	this->current_tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	/*
	 * Disable software flow control
	 */
	this->current_tio.c_iflag &= ~(IXON | IXOFF | IXANY);
	/*
	 * Disable all input processing.  This fucker got me.  The port was automatically converting all 0x0D to 0x0A
	 */
	this->current_tio.c_iflag &= ~(BRKINT | INLCR | ICRNL | IUCLC | BRKINT | IMAXBEL | IGNCR);
	this->current_tio.c_iflag |= (IGNBRK);
	/*
	 * disable postprocess output
	 */
	this->current_tio.c_oflag &= ~OPOST;
	/*
	 * Don't care about minimum character counts or timeouts.
	 */
	this->current_tio.c_cc[VMIN] = 0;
	this->current_tio.c_cc[VTIME] = 0;
	/*
	 * Enable hardware control
	 */
	this->current_tio.c_cflag |= CRTSCTS;

	if(tcsetattr(this->serial_fd, TCSANOW, &this->current_tio) < 0)
	{
		LOG_ERROR_P(create_perror_string("Serial port attributes"));
		return ENUM_ERRORS::ERR_ATTRIBUTES;
	}

	return ENUM_ERRORS::ERR_NONE;
}

ENUM_ERRORS SER_IO_COMM::init(void)
{
	LOG_INFO_P("Operating on serial port:" + this->tty_dev);

	if(!this->try_lock_serial())
	{
		LOG_ERROR_P("Failed to obtain lock on serial port.");
		return ENUM_ERRORS::ERR_FAIL_LOCK;
	}

	ENUM_ERRORS rc;

	if((rc = this->serial_port_open()) != ENUM_ERRORS::ERR_NONE)
	{
		LOG_ERROR_P("Failed to open serial port.");
		return rc;
	}

	LOG_DEBUG_P("Init successful");
	return ENUM_ERRORS::ERR_NONE;
}

bool SER_IO_COMM::write_buffer(const unsigned char* _buffer, size_t _length)
{
	ssize_t bytes_written = 0;
	ssize_t rc;

	while(this->abort_thread == false)
	{
		if(!this->clear_to_send())
		{
			continue;
		}

		rc = write(this->serial_fd, _buffer + bytes_written, _length - bytes_written);

		if(rc < 0)
		{
			LOG_ERROR_P(create_perror_string(this->tag + ": Failed to write message to IO board. rc < 0"));
			return false;
		}

		bytes_written += rc;

		if((size_t) bytes_written == _length)
		{
			break;
		}
	}

	if(_length != (size_t) bytes_written)
	{
		LOG_ERROR_P("Write loop aborted before writing full buffer.  bytes written: " + num_to_str(bytes_written) +"; buffer length: " + num_to_str(_length));
		return false;
	}
	else
	{
		return true;
	}
}

bool SER_IO_COMM::add_do_status(size_t _idx)
{
	this->state_cache.add_do_status(this->active_table->table[_idx][5]);
	return true;
}

bool SER_IO_COMM::add_pmic_status(size_t _idx)
{
	this->state_cache.add_pmic_status(this->active_table->table[_idx][5]);
	return true;
}

bool SER_IO_COMM::add_cache_values(size_t _idx,unsigned char _level)
{
	BOARD_STATE_CACHE::CAL_VALUE_ADDER_PTR adder_ptr;
	unsigned char* line = this->active_table->table[_idx];
	uint16_t length = ASSEMBLE_16INT(line[RESP_HEAD_LEN_LSB_IDX], line[RESP_HEAD_LEN_MSB_IDX]);
	size_t result_index = 0;

	/*
	XXX - Need to reimplement
	if(length != GC_IO_AI_COUNT * 2)
	{
		LOG_ERROR_P("Invalid payload length for cache value response.  Expecting: " + num_to_str(GC_IO_AI_COUNT * 2) + ", received: " + num_to_str(length));
		return false;
	}
	*/

	if(_level == 1)
	{
		adder_ptr = &BOARD_STATE_CACHE::add_l1_cal_value;
	}
	else if(_level == 2)
	{
		adder_ptr = &BOARD_STATE_CACHE::add_l2_cal_value;
	}
	else
	{
		LOG_ERROR_P("Invalid cache level parameter supplied.");
		return false;
	}

	for(size_t i = 0; (i < length && result_index < GC_IO_AI_COUNT); i += 2)
	{
		adder_ptr(this->state_cache,result_index, ASSEMBLE_16INT(line[RESP_HEAD_SIZE + i], line[RESP_HEAD_SIZE + i + 1]));
		result_index += 1;
	}

	return true;
}

bool SER_IO_COMM::add_boot_count(size_t _idx)
{
	unsigned char* line = this->active_table->table[_idx];
	uint16_t length = ASSEMBLE_16INT(line[RESP_HEAD_LEN_LSB_IDX], line[RESP_HEAD_LEN_MSB_IDX]);

	if(length != 10)
	{
		LOG_ERROR_P("Invalid payload length received for boot count response.");
		return false;
	}

	this->state_cache.set_boot_count(ASSEMBLE_16INT(line[6], line[7]));
	return true;
}

bool SER_IO_COMM::add_ai_result(size_t _line_index)
{
	unsigned char* line = this->active_table->table[_line_index];
	uint16_t length = ASSEMBLE_16INT(line[RESP_HEAD_LEN_LSB_IDX], line[RESP_HEAD_LEN_MSB_IDX]);
	length = (uint16_t)(length - 3); 	// This is goddamn ridiculous
	size_t result_index = 0;

	/*
	We're expecting RESP_HEAD_SIZE of header, 16 bytes of payload, and 2 bytes of checksum
	*/

	for(size_t i = 0; (i < length && result_index < GC_IO_AI_COUNT); i += 2)
	{
		this->state_cache.add_adc_value(result_index, ASSEMBLE_16INT(line[RESP_HEAD_SIZE + i], line[RESP_HEAD_SIZE + i + 1]));
		result_index = result_index + 1;
	}

	return true;
}

void SER_IO_COMM::set_active_table_line_blank(size_t _idx)
{
	this->active_table->table[_idx][0] = 0xFF;
	this->active_table->table[_idx][1] = 0xFF;
	return;
}

void SER_IO_COMM::process_binary_message(size_t _idx)
{
	//uint8_t cmd = this->active_table->table[_idx][1];
	ENUM_BOARD_COMMANDS cmd = static_cast < ENUM_BOARD_COMMANDS >(this->active_table->table[_idx][1]);
	//uint8_t status = this->active_table->table[i][2];
	uint16_t length = ASSEMBLE_16INT(this->active_table->table[_idx][RESP_HEAD_LEN_LSB_IDX], this->active_table->table[_idx][RESP_HEAD_LEN_MSB_IDX]);
	uint16_t * data_ptr = (uint16_t *)this->active_table->table[_idx];
	uint16_t chksum = checksum(data_ptr,(length  + RESP_HEAD_SIZE)/ 2);

	if(chksum != 0)
	{
		LOG_ERROR_P("Message failed checksum check: " + num_to_str(chksum) + ", in message: " + num_to_str(data_ptr[4]));
		LOG_ERROR_P("Message length: " + num_to_str(length));
		LOG_ERROR_P(buffer_to_hex(this->active_table->table[_idx],10));
		goto _end;
	}

	//LOG_DEBUG_P("Command: " + to_string(cmd) + ", status: " + to_string(status) + ", length: " + to_string(length));

	switch(cmd)
	{
		case CMD_ID_RESET_BOARD:
		{
			// This should never happen.
			LOG_ERROR_P("We received a response of type CMD_ID_RESET_BOARD??  How is that possible?")
			LOG_ERROR_P(buffer_to_hex(this->active_table->table[_idx],RESP_HEAD_SIZE + length));
			this->set_active_table_line_blank(_idx);
			break;
		}

		case CMD_ID_GET_AI_STATUS:
		{
			if(length % 2 != 0)
			{
				LOG_ERROR_P("Payload length for response to CMD_ID_GET_AI_SATATUS is not a multiple of two [" + num_to_str(length) + "].  Clearing offending line.");
				this->set_active_table_line_blank(_idx);
				return;
			}

			this->add_ai_result(_idx);
			break;
		}

		case CMD_ID_GET_DO_STATUS:
		{
			this->add_do_status(_idx);
			break;
		}

		case CMD_ID_SET_DO_STATUS:
		{
			//LOG_ERROR_P( "Accepting response to CMD_ID_SET_DO_STATUS is not yet implemented." );
			/*
			 * There's really nothing for us to do with a response.
			 */
			this->set_active_table_line_blank(_idx);
			break;
		}

		case CMD_ID_GET_PMIC_STATUS:
		{
			//LOG_ERROR_P( "Accepting response to CMD_ID_GET_PMIC_STATUS is not yet implemented." );
			/*
			 * What can we possibly do about a response.
			 */
			this->add_pmic_status(_idx);
			break;
		}

		case CMD_ID_SET_PMIC_STATUS:
		{
			//LOG_ERROR_P( "Accepting response to CMD_ID_SET_PMIC_STATUS is not yet implemented." );
			this->set_active_table_line_blank(_idx);
			break;
		}

		case CMD_ID_GET_L1_CAL_VALS:
		{
			this->add_cache_values(_idx,1);
			break;
		}

		case CMD_ID_GET_L2_CAL_VALS:
		{
			this->add_cache_values(_idx,2);
			break;
		}

		case CMD_ID_SET_L1_CAL_VALS:
		{
			//LOG_ERROR_P( "Accepting response to CMD_ID_SET_L1_CAL_VALS is not yet implemented." );
			break;
		}

		case CMD_ID_SET_L2_CAL_VALS:
		{
			//LOG_ERROR_P("Accepting response to CMD_ID_SET_L2_CAL_VALS is not yet implemented.");
			break;
		}

		case CMD_ID_GET_BOOT_COUNT:
		{
			this->add_boot_count(_idx);
			break;
		}

		case CMD_ID_GET_CONFIRM_OUTPUT:
		{
			// Do nothing
			break;
		}

		default:
		{
			LOG_ERROR_P("Unrecognized command: " + to_string(cmd));
			break;
		}
	}

_end:
	this->set_active_table_line_blank(_idx);
	return;
}

token_vector SER_IO_COMM::tokenize_protocol_line(size_t _line_index)
{
	token_vector ret;
	const unsigned char* line = this->active_table->table[_line_index];
	ssize_t str_len = strlen((const char*) line);
	ssize_t left_idx = find_last_index_of(line, '|', str_len);
	ssize_t right_idx = 0;

	if(left_idx < 0)
	{
		LOG_ERROR_P("Malformed protocol message: " + string((const char*) line));
		return ret;
	}

	right_idx = find_index_of(line, '.', left_idx + 1, str_len);

	while(1)
	{
		if(left_idx + 1 != right_idx)
		{
			ret.push_back(token(left_idx + 1, right_idx));
		}
		else
		{
			break;
		}

		left_idx = right_idx;

		if(left_idx == str_len)
		{
			break;
		}

		right_idx = find_index_of(line, '.', right_idx + 1, str_len);

		if(right_idx < 0)
		{
			right_idx = str_len;
		}
	}

	return ret;
}

vector<string> SER_IO_COMM::create_protocol_line_tokens(const token_vector& _vect, size_t _idx)
{
	const unsigned char* _line = this->active_table->table[_idx];
	const string line((const char*) _line);
	vector<string>ret;

	for(token_vector::const_iterator i = _vect.begin(); i != _vect.end(); ++i)
	{
		string s = line.substr(i->first, i->second - i->first);
		trim(s);
		ret.push_back(s);
	}

	return ret;
}

void SER_IO_COMM::process_protocol_message(size_t _idx)
{
	token_vector token_indexes = this->tokenize_protocol_line(_idx);

	if(token_indexes.size() < 1)
	{
		return;
	}

	/*
	 * XXX - crash on malformed messages here
	 */
	vector<string> tokens = this->create_protocol_line_tokens(token_indexes, _idx);

	if(tokens[0] == "F IC" && tokens[1] == "IC UP")
	{
		LOG_DEBUG_P("Board reset sensed.");
		this->board_has_reset = true;
	}

	return;
}

void SER_IO_COMM::process_text_message(size_t _idx)
{
	LOG_DEBUG_P(string((char*)(this->active_table->table[_idx])));

	if(this->active_table->table[_idx][2] == '9')
	{
		this->process_protocol_message(_idx);
	}

	this->set_active_table_line_blank(_idx);
	return;
}

bool SER_IO_COMM::digest_line_table(void)
{
	for(size_t i = 0; i < this->active_table->index; i++)
	{
		if(this->active_table->table[i][0] == 0xFF && this->active_table->table[i][1] == 0xFF)
		{
			/*
			 * Blank line
			 */
			continue;
		}
		else if(this->active_table->table[i][0] == 0x10)
		{
			/*
			 * Binary message
			 */
			this->process_binary_message(i);
		}
		else if(this->active_table->table[i][0] >= 32 && this->active_table->table[i][0] <= 126)
		{
			this->process_text_message(i);
		}
		else
		{
			LOG_ERROR_P("Line at index " + to_string(i) + " was unrecognized.  Clearing offending line.");
			LOG_DEBUG_P("\n" + buffer_to_hex(this->active_table->table[i],16));
			this->set_active_table_line_blank(i);
		}
	}

	this->compact_line_table();
	return true;
}

bool SER_IO_COMM::swap_two_line_table_lines(size_t _target_idx, size_t _source_idx)
{
	if(_target_idx >= GC_SERIAL_LINE_TABLE_ENTRIES || _source_idx >= GC_SERIAL_LINE_TABLE_ENTRIES)
	{
		LOG_ERROR_P("One of the supplied indexes is >= GC_SERIAL_LINE_TABLE_ENTRIES.");
		return false;
	}

	unsigned char* temp = this->active_table->table[_target_idx];
	this->active_table->table[_target_idx] = this->active_table->table[_source_idx];
	this->active_table->table[_source_idx] = temp;
	return true;
}

size_t SER_IO_COMM::find_next_nonblank_line_table_line(size_t _start_idx) const
{
	if(_start_idx >= GC_SERIAL_LINE_TABLE_ENTRIES)
	{
		LOG_ERROR_P("Supplied line index >= GC_SERIAL_LINE_TABLE_ENTRIES.");
		return _start_idx;
	}

	for(size_t i = _start_idx; i < GC_SERIAL_LINE_TABLE_ENTRIES; i++)
	{
		if(this->is_line_table_line_blank(_start_idx))
		{
			return i;
		}
	}

	/*
	 * Did not find a next blank line.
	 */
	return _start_idx;
}

bool SER_IO_COMM::is_line_table_line_blank(size_t _idx) const
{
	if(_idx >= GC_SERIAL_LINE_TABLE_ENTRIES)
	{
		LOG_ERROR_P("Supplied line index >= GC_SERIAL_LINE_TABLE_ENTRIES.");
		return false;
	}

	if(this->active_table->table[_idx][0] == 0xFF && this->active_table->table[_idx][1] == 0xFF)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SER_IO_COMM::compact_line_table(void)
{
	size_t non_blank_lines = 0;
	size_t current_line_index = this->active_table->index;

	for(size_t i = 0; i < GC_SERIAL_LINE_TABLE_ENTRIES; i++)
	{
		if(this->is_line_table_line_blank(i))
		{
			//LOG_DEBUG_P( "Found blank line @ " + to_string( i ) );
			if(i != GC_SERIAL_LINE_TABLE_ENTRIES - 1)
			{
				size_t nb_idx = this->find_next_nonblank_line_table_line(i + 1);

				if(nb_idx == i)
				{
					/*
					 * Failed to find next nonblank line?  All lines after this one are blank?
					 */
					break;
				}
				else
				{
					/*
					 * Found a next non blank line.
					 */
					if(!this->swap_two_line_table_lines(i, nb_idx))
					{
						LOG_ERROR_P("Failed to swap lines. Target: " + to_string(i) + ", source: " + to_string(nb_idx));
					}
				}
			}
		}
		else
		{
			non_blank_lines += 1;
		}
	}

	this->active_table->index = non_blank_lines;

	if(this->active_table->index == GC_SERIAL_LINE_TABLE_ENTRIES)
	{
		/*
		 * All line entries are full.
		 */
		this->active_table->index = current_line_index;
	}

	return;
}

int SER_IO_COMM::assemble_serial_data(void)
{
	size_t copy_len = 0;

	for(; this->buffer_context.buffer_work_index < this->buffer_context.buffer_idx; this->buffer_context.buffer_work_index++)
	{
		unsigned char work_char = this->buffer[this->buffer_context.buffer_work_index];

		if(work_char == 0x10)   // begin of binary marker record.
		{
			/*
			Once we have the marker of the binary message, we enter enter binary message mode.
			*/
			this->buffer_context.bin_msg_start_index = this->buffer_context.buffer_work_index;
			this->buffer_context.in_bin_message = true;
			//LOG_DEBUG_P("Binary marker @ " + num_to_str(this->buffer_context.buffer_work_index));
			continue;
		}
		else if(this->buffer_context.in_bin_message == true)
		{
			/*
			 *
			 */
			size_t expected_index = this->buffer_context.bin_msg_start_index + RESP_HEAD_SIZE;

			if(this->buffer_context.buffer_idx + this->buffer_context.bin_msg_start_index >= expected_index)   // make sure we have enough data in the buffer for the standard preamble
			{
				uint16_t length = ASSEMBLE_16INT(this->buffer[this->buffer_context.bin_msg_start_index + RESP_HEAD_LEN_LSB_IDX], this->buffer[this->buffer_context.bin_msg_start_index + RESP_HEAD_LEN_MSB_IDX]);

				//LOG_DEBUG_P(buffer_to_hex(this->buffer + this->buffer_context.bin_msg_start_index,8));
				//LOG_DEBUG_P("Expected message length: " + num_to_str(length));

				if(length >= GC_SERIAL_BUFF_SIZE || length > 32)
				{
					LOG_ERROR_P("Weird message size: " + num_to_str(length) + "; nuking context");
					LOG_DEBUG_P("\n" + buffer_to_hex(this->buffer + this->buffer_context.bin_msg_start_index ,8));
					this->reset_buffer_context();
					memset(this->buffer,0x00,GC_SERIAL_BUFF_SIZE);
					break;
				}

				//LOG_DEBUG_P("Expected message length: " + num_to_str(length));

				if((this->buffer_context.bin_msg_start_index +  RESP_HEAD_SIZE + length) == GC_SERIAL_BUFF_SIZE)
				{
					LOG_ERROR_P("Possible buffer overflow.  Record length: " + to_string(this->buffer_context.buffer_work_index + RESP_HEAD_SIZE + length) + ", GC_SERIAL_BUFF_SIZE: " + to_string(GC_SERIAL_BUFF_SIZE) + ".  Nuking context.");
					this->reset_buffer_context();
					memset(this->buffer,0x00,GC_SERIAL_BUFF_SIZE);
					break;
				}

				expected_index = this->buffer_context.bin_msg_start_index + RESP_HEAD_SIZE + length;

				if(this->buffer_context.buffer_idx >= expected_index)
				{
					//LOG_DEBUG_P("Message start index: " + num_to_str(this->buffer_context.bin_msg_start_index) + ", message end index: " + num_to_str(expected_index) + ", buffer index: " + num_to_str(this->buffer_context.buffer_idx));
					//LOG_DEBUG_P("\n" + buffer_to_hex(this->buffer,this->buffer_context.buffer_idx,false));


					this->add_to_active_table(this->buffer + this->buffer_context.bin_msg_start_index, length + RESP_HEAD_SIZE);   // the binary record marker, preamble, and data
					//this->buffer_context.buffer_work_index = this->buffer_context.bin_msg_start_index + length + 4;
					/*
					We just processed a binary message.
					Need to remove the chunk of data from the buffer and adjust the buffer_work_index accordingly.
					*/
					unsigned char* source_ptr = this->buffer + expected_index;
					unsigned char* dest_ptr = this->buffer + this->buffer_context.bin_msg_start_index;
					size_t move_length = this->buffer_context.buffer_idx - expected_index;

					if(source_ptr < this->buffer || source_ptr > (this->buffer + GC_SERIAL_BUFF_SIZE))
					{
						LOG_ERROR_P("Source pointer weirdness.  Nuking context.");
						this->reset_buffer_context();
						memset(this->buffer,0x00,GC_SERIAL_BUFF_SIZE);
						break;
					}

					if(dest_ptr < this->buffer || (dest_ptr + move_length) > (this->buffer + GC_SERIAL_BUFF_SIZE) || move_length > GC_SERIAL_BUFF_SIZE)
					{
						LOG_ERROR_P("Destination pointer weirdness.  Nuking context.");
						this->reset_buffer_context();
						memset(this->buffer,0x00,GC_SERIAL_BUFF_SIZE);
						break;
					}

					//LOG_DEBUG_P("Offset: " + num_to_str(expected_index) + ", buffer index: " + num_to_str(this->buffer_context.buffer_idx));
					//LOG_DEBUG_P("Move length: " + num_to_str(move_length));

					if(move_length > 0)
					{
						memmove(dest_ptr,source_ptr,move_length);
					}

					//this->buffer_context.buffer_idx = this->buffer_context.bin_msg_start_index + move_length;

					this->buffer_context.buffer_idx = (this->buffer_context.buffer_idx - expected_index);

					//LOG_DEBUG_P("Buffer index after move: " + num_to_str(this->buffer_context.buffer_idx));

					this->buffer_context.buffer_work_index = 0 ;
					this->buffer_context.in_bin_message = false;
					this->buffer_context.bin_msg_start_index = 0;


					//LOG_DEBUG_P( "**************************************" );

					/*
					* Abandon loop until next invocation.
					* This is due to the buffer_work_index getting incremented at next iteration.  We can't prevent it without doing gymnastics
					*/
					break;

				}
				else
				{
					/*
					 * If we don't have enough data for the whole record.  Wait for more data.
					 */
					break;
				}
			}
			else
			{
				/*
				 * If we don't have enough data for the preamble.  Wait for more data.
				 */
				break;
			}

			if(this->buffer_context.buffer_work_index == this->buffer_context.buffer_idx)
			{
				/*
				 * Whole buffer has been consumed.  Reset all of the indexes.
				 */
				this->reset_buffer_context();
				break;
			}
			else
			{
				//LOG_DEBUG_P( "work_index = " + to_string(this->buffer_context.buffer_work_index) + " buffer_idx = " + to_string(this->buffer_context.buffer_idx));
			}

			continue;
		} // we're inside a binary message
		else if(work_char ==  '\n' || work_char == '\r')
		{
			/*
			 * Text data
			 *
			 * This block has to be second because \n and \r are valid in binary message bytes
			 */
			if(this->buffer_context.buffer_work_index == 0)
			{
				this->buffer_context.buffer_start_index += 1;
				continue;
			}
			else if(this->buffer_context.buffer_start_index == 0)
			{
				/*
				 * A \n or \r are not the very first character, but no previous instances have been encountered.
				 */
				copy_len = this->buffer_context.buffer_work_index;
			}
			else
			{
				/*
				 * Number of characters is current character minus previous start minus one to skip the current terminator.
				 */
				copy_len = this->buffer_context.buffer_work_index - this->buffer_context.buffer_start_index;
			}

			if(copy_len > 0)
			{
				if(copy_len >= GC_SERIAL_BUFF_SIZE)
				{
					LOG_ERROR_P("Text message length (" + num_to_str(copy_len) + ") is greater than GC_SERIAL_BUFF_SIZE.  Nuking context.");
					this->reset_buffer_context();
					memset(this->buffer,0x00,GC_SERIAL_BUFF_SIZE);
					break;
				}

				this->add_to_active_table(buffer + this->buffer_context.buffer_start_index, copy_len);
				this->buffer_context.buffer_start_index = this->buffer_context.buffer_work_index + 1; // Advance past the current line terminator
			}

			if(this->buffer_context.buffer_start_index == this->buffer_context.buffer_idx)
			{
				/*
				 * Whole buffer has been consumed.  Reset all of the indexes.
				 */
				this->reset_buffer_context();
				break;
			}
			else
			{
				//LOG_DEBUG_P( "buffer_work_index = " + to_string( this->buffer_context.buffer_work_index ) + ", buffer_index = " + to_string( this->buffer_context.buffer_idx ) + ", start_index = " + to_string(this->buffer_context.buffer_start_index));
			}
		}
	}

	return 1;
}

void SER_IO_COMM::clear_active_table_current_line(void)
{
	memset(this->active_table->table[this->active_table->index], 0, GC_SERIAL_BUFF_SIZE);
	return;
}

void SER_IO_COMM::add_to_active_table(const unsigned char* _buffer, size_t _length)
{
	this->clear_active_table_current_line();
	memcpy(this->active_table->table[this->active_table->index], _buffer, _length);

	//std::string b = buffer_to_hex(this->active_table->table[this->active_table->index],_length);
	//LOG_DEBUG_P(b);

	this->active_table->index += 1;

	if(this->active_table->index == GC_SERIAL_LINE_TABLE_ENTRIES)
	{
		LOG_DEBUG_P("Active table wrap around");
		this->active_table->index = 0;
	}

	return;
}

void SER_IO_COMM::handle_hung_board()
{
	LOG_DEBUG_P("Handling hung board.");
	this->board_has_reset = false;
	this->reset_buffer_context();
	this->serial_port_close();
	/*
	timespec ts;
	ts.tv_sec = 2;
	ts.tv_nsec = 0;
	this->nsleep(&ts);
	 */
	this->serial_port_open();
	this->cmd_reset_board();
	return;
}

bool SER_IO_COMM::main_event_loop(void)
{
	size_t loop_counter = 0;
	size_t zero_fd_counter = 0;
	struct pollfd fds;
	this->reset_sleep_timespec(GC_SERIAL_THREAD_SLEEP);
	/*
	 * Reset board as a first order of business so that we can be sure of its state.
	 */

	if(this->in_debug_mode)
	{
		this->board_has_reset = true;
	}
	else
	{
		this->cmd_reset_board();
	}

	/*
	 * The goal here is to get the data out of the UART as fast as possible because the IO board comm chip blocks on hardware flow control.
	 * Next priority is assembling the data in the input buffer and shoving it out into the line buffer.  The incomming data assembler is not super robust.  It gets confused
	 * by fragmented data.  Hence the use of poll with a 10mSec timeout.  Any faster than that and the assembly function gets confused.
	 *
	 * 1 -- read data from UART and shove into buffer
	 * 2 -- assemble the data and pull it out of the buffer and shove it into the line buffer.
	 * 3 -- read and remove data from the line buffer
	 * 4 -- send out the waiting message.
	 */
	while(this->abort_thread == false)
	{
		this->obtain_lock();
		fds.fd = this->serial_fd;
		fds.events = POLLIN | POLLPRI | POLLERR | POLLHUP;
		int fds_ready_num = poll(&fds, 1, GC_SERIAL_THREAD_POLL_TIMEOUT);

		if(fds_ready_num == 0)
		{
			/*
			 * Timeout while waiting for the serial file descriptor to get data
			 */
			zero_fd_counter += 1;

/*
			if(this->buffer_context.buffer_idx > 0)
			{
				this->assemble_serial_data();
			}

			this->digest_line_table();
*/

			if(loop_counter >= GC_SERIAL_THREAD_UPDATE_INTERVAL)
			{
				if(loop_counter > GC_SERIAL_THREAD_UPDATE_INTERVAL)
				{
					//LOG_WARNING_P("Delayed state refresh.  Counter: " + num_to_str(loop_counter));
				}

				if(this->board_has_reset)
				{
					//LOG_DEBUG_P("Refreshing.");

					this->cmd_refresh_analog_inputs();
					this->cmd_refresh_digital_outputs();
					this->cmd_refresh_pmic_status();
					//this->cmd_confirm_output_state();
					//this->cmd_refresh_l1_cal_values();
					//this->cmd_refresh_l2_cal_values();
					//this->cmd_refresh_boot_count();
				}
				else
				{
					//LOG_DEBUG_P("Waiting for board to reset.");
				}

				loop_counter = 0;
			}
		}
		else if(fds_ready_num < 0)
		{
			/*
			 * Error encountered by 'poll'
			 * XXX - What do we do here?  Ignore?
			 */
			LOG_ERROR_P(create_perror_string("Poll"));
		}
		else
		{
			zero_fd_counter = 0;

			/*
			 * A file descriptor is ready for action
			 */
			if(fds.revents & POLLIN)
			{
				this->drain_serial();

				this->assemble_serial_data();
				this->digest_line_table();
			}
			else
			{
				/*
				 * We don't know how to handle anything other than incomming data.
				 */
				LOG_ERROR_P("Don't know what to do when poll.revents is not POLLIN");
			}
		}

		if(!this->release_lock())
		{
			break;
		}

		if(zero_fd_counter == 1000)
		{
			if(!this->in_debug_mode)
			{
				this->handle_hung_board();
			}

			zero_fd_counter = 0;
		}

		loop_counter += 1;
		/*
		 * Sleep for 10 microseconds to give other threads a chance to get at the data.
		 */
		this->reset_sleep_timespec(GC_SERIAL_THREAD_SLEEP);
		this->nsleep(nullptr);
	}

	return true;
}

bool SER_IO_COMM::write_event_loop(void) throw(LOCK_ERROR)
{
	std::queue<OUTGOING_MESSAGE> work_queue;

	/*
	 * This event loop runs in a different thread than main_event_loop
	 */
	while(this->abort_thread == false)
	{
		if(this->outgoing_messages->wait_for_signal() == false)
		{
			/*
			 * Timed out on conditional.
			 * We time and loop again in order to be able to sense the global exit and abort thread flags.
			 */
			continue;
		}

		/*
		 * Conditional was triggered.  We have the mutex
		 */
		this->outgoing_messages->swap_message_queue(&work_queue);
		/*
		 * We only need the lock long enough to copy the message queue.
		 * We release the lock as fast as we can so that other threads may continue to add messages to the queue.
		 */
		this->outgoing_messages->put_lock();

		while(!work_queue.empty())
		{
			const OUTGOING_MESSAGE& msg = work_queue.front();

			if(!this->write_buffer(msg.message.get(), msg.message_length))
			{
				LOG_ERROR_P("Failed to write whole message.");
				this->abort_thread = true;
				break;
			}

			work_queue.pop();
		}
	}

	return true;
}

bool SER_IO_COMM::send_message(const unsigned char* _message, size_t _length)
{
	/*
	 * We don't need the main lock for this method since there's a separate writer thread.
	 */
	if(_length >= GC_SERIAL_BUFF_SIZE)
	{
		LOG_ERROR_P("Tried to send message that would overflow buffer.");
		return false;
	}

	OUTGOING_MESSAGE msg((const unsigned char*) _message, _length);
	this->outgoing_messages->add_message(msg);
	return true;
}

void SER_IO_COMM::swap_tables(void)
{
	/*
	 * We're making an assumption that the object is locked when this is invoked.
	 */
	if(this->active_table == &(this->table_a))
	{
		this->active_table = &(this->table_b);
		this->passive_table = &(this->table_a);
	}
	else
	{
		this->active_table = &(this->table_a);
		this->passive_table = &(this->table_b);
	}

	this->active_table->index = 0;
	return;
}

void SER_IO_COMM::reset_buffer_context(void)
{
	memset(&this->buffer_context, 0, sizeof(ST_SERIAL_BUFFER_CONTEXT));
	return;
}

bool SER_IO_COMM::get_dac_cache(ADC_CACHE_ENTRY(&_dest)[GC_IO_STATE_BUFFER_DEPTH][GC_IO_AI_COUNT])
{
	this->obtain_lock();
	this->state_cache.get_adc_cache(_dest);
	this->release_lock();
	return true;
}

bool SER_IO_COMM::get_do_cache(DO_CACHE_ENTRY(& _dest)[GC_IO_STATE_BUFFER_DEPTH])
{
	this->obtain_lock();
	this->state_cache.get_do_cache(_dest);
	this->release_lock();
	return true;
}

bool SER_IO_COMM::get_pmic_cache(PMIC_CACHE_ENTRY(& _dest)[GC_IO_STATE_BUFFER_DEPTH])
{
	this->obtain_lock();
	this->state_cache.get_pmic_cache(_dest);
	this->release_lock();
	return true;
}

bool SER_IO_COMM::get_latest_state_values(BOARD_STATE_CACHE& _target)
{
	this->obtain_lock();
	_target = this->state_cache;
	this->release_lock();
	return true;
}

bool SER_IO_COMM::get_latest_adc_values(ADC_CACHE_ENTRY(& _dest)[GC_IO_AI_COUNT])
{
	this->obtain_lock();
	this->state_cache.get_latest_adc_values(_dest);
	this->release_lock();
	return true;
}

bool SER_IO_COMM::get_latest_do_status(DO_CACHE_ENTRY& _dest)
{
	this->obtain_lock();
	this->state_cache.get_latest_do_status(_dest);
	this->release_lock();
	return true;
}

bool SER_IO_COMM::get_latest_pmic_status(PMIC_CACHE_ENTRY& _dest)
{
	this->obtain_lock();
	this->state_cache.get_latest_pmic_status(_dest);
	this->release_lock();
	return true;
}

bool SER_IO_COMM::cmd_set_do_status(uint8_t _status)
{
	unsigned char buffer [5];
	buffer[0] = '@';
	buffer[1] = 0x00;	// length MSB
	buffer[2] = 0x02;	// length LSB
	buffer[3] = CMD_ID_SET_DO_STATUS;
	buffer[4] = _status;
	return this->send_message((const unsigned char*)(&buffer), 5);
}

bool SER_IO_COMM::cmd_set_pmic_status(uint8_t _status)
{
	unsigned char buffer [5];
	buffer[0] = '@';
	buffer[1] = 0x00;	// length MSB
	buffer[2] = 0x02;	// length LSB
	buffer[3] = CMD_ID_SET_PMIC_STATUS;
	buffer[4] = _status;
	return this->send_message((const unsigned char*)(&buffer), 5);
}

bool SER_IO_COMM::cmd_refresh_l1_cal_values(void)
{
	unsigned char buffer [4];
	buffer[0] = '@';
	buffer[1] = 0x00;	// length MSB
	buffer[2] = 0x01;	// length LSB
	buffer[3] = CMD_ID_GET_L1_CAL_VALS;
	return this->send_message((const unsigned char*)(&buffer), 4);
}

bool SER_IO_COMM::cmd_refresh_l2_cal_values(void)
{
	unsigned char buffer [4];
	buffer[0] = '@';
	buffer[1] = 0x00;	// length MSB
	buffer[2] = 0x01;	// length LSB
	buffer[3] = CMD_ID_GET_L2_CAL_VALS;
	return this->send_message((const unsigned char*)(&buffer), 4);
}

bool SER_IO_COMM::cmd_refresh_boot_count(void)
{
	unsigned char buffer [4];
	buffer[0] = '@';
	buffer[1] = 0x00;	// length MSB
	buffer[2] = 0x01;	// length LSB
	buffer[3] = CMD_ID_GET_BOOT_COUNT;
	return this->send_message((const unsigned char*)(&buffer), 4);
}

bool SER_IO_COMM::cmd_confirm_output_state(void)
{
	unsigned char buffer [4];
	buffer[0] = '@';
	buffer[1] = 0x00;	// length MSB
	buffer[2] = 0x01;	// length LSB
	buffer[3] = CMD_ID_GET_CONFIRM_OUTPUT;
	return this->send_message((const unsigned char*)(&buffer), 4);
}

bool SER_IO_COMM::cmd_set_l1_calibration_values(const CAL_VALUE_ARRAY& _values)
{
	return this->cmd_set_calibration_values(CMD_ID_SET_L1_CAL_VALS,_values);
}
bool SER_IO_COMM::cmd_set_l2_calibration_values(const CAL_VALUE_ARRAY& _values)
{
	return this->cmd_set_calibration_values(CMD_ID_SET_L2_CAL_VALS,_values);
}

bool SER_IO_COMM::cmd_set_calibration_values(unsigned char _cmd,const CAL_VALUE_ARRAY& _values)
{
	std::vector<std::string> sv;
	convert_vector_to_string(_values,sv);
	//LOG_DEBUG_P(join_vector(sv,':'));
#define BUFF_LENGTH 4 + (GC_IO_AI_COUNT * 2)
	unsigned char buffer [BUFF_LENGTH];
	buffer[0] = '@';
	buffer[1] = 0x00;	// length MSB
	buffer[2] = 0x11;	// length LSB
	// 1 + (GC_IO_AI_COUNT * 2)
	// Theres eight inputs and each input takes two bytes to describe the calibration offset.  Plus one for the call index hence:
	// 1 + (8 * 2)
	buffer[3] = _cmd;
	unsigned char* ob = buffer;
	unsigned char* b = ob;
	b += 4;

	for(size_t i=0; i<GC_IO_AI_COUNT; i++)
	{
		uint16_t v = _values[i];
		unsigned char msb = (unsigned char)((v >> 8) & 0x00FF);
		unsigned char lsb = (unsigned char)((v) & 0x00FF);
		//LOG_DEBUG_P("Value(" + num_to_str(i) + "): " + num_to_str(v) + "; MSB:" + num_to_str(msb) + ", LSB:" + num_to_str(lsb));
		b[i * 2] = msb;
		b[(i * 2) + 1] = lsb;
	}

	//LOG_DEBUG_P("Sending binary set calibration message");
	//LOG_DEBUG_P(buffer_to_hex(buffer,BUFF_LENGTH));
	return this->send_message((const unsigned char*)(&buffer),BUFF_LENGTH);
	//return true;
}

bool SER_IO_COMM::cmd_reset_board(void)
{
	this->board_has_reset = false;
	return this->send_message((const unsigned char*) "@\x00\x01\x00", 4);
	//return true;
}

bool SER_IO_COMM::cmd_refresh_analog_inputs(void)
{
	return this->send_message((const unsigned char*) "@\x00\x01\x01", 4);
}

bool SER_IO_COMM::cmd_refresh_digital_outputs(void)
{
	return this->send_message((const unsigned char*) "@\x00\x01\x02", 4);
}

bool SER_IO_COMM::cmd_refresh_pmic_status(void)
{
	return this->send_message((const unsigned char*) "@\x00\x01\x04", 4);
}

bool SER_IO_COMM::cmd_refresh_help(void)
{
	return this->send_message((const unsigned char*) "?\n", 2);
}
