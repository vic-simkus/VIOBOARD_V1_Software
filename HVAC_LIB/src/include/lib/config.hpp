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


#ifndef CONFIG_H_
#define CONFIG_H_

/**
 * The name of the user that the server will assume on startup.
 */
#define GC_PROC_USER "nobody"

/**
 * Name of the group that the server will assume on startup.
 */
#define GC_PROC_GROUP "nogroup"

/**
 * Unix domain socket that the server will create and bind to or client will connect to.
 */
#define GC_LOCAL_COMMAND_SOCKET "/tmp/bbb_hvac"

/**
 * Buffer size for various read/write operations.  Should not need to be changed.
 */
#define GC_BUFFER_SIZE 4096

/**
 * Time in seconds that the server process client thread will use to timeout from the 'select' call.
 * The actual select timeout will be GC_CLIENT_THREAD_SELECT_TIME + GC_CLIENT_THREAD_SELECT_TIME * 1000 / GC_CLIENT_PING_DIVIDER in order to
 * allow the keep alive PINGs/PONGs to timeout between timeouts
 */
#define GC_CLIENT_THREAD_SELECT_TIME 1

/**
 * Interval with which to send the ping messages to the client.  GC_CLIENT_THREAD_SELECT_TIME * GC_CLIENT_PING_DIVIDER is the number of seconds between each ping message.
 * This interval will also be used to determine if the remote has gone off the deep end and stopped responding to network comms.
 * In such a case the connection will be closed and client thread terminated.
 */
#define GC_CLIENT_PING_DIVIDER 5

/**
 * Incoming message queue size
 */
#define GC_INCOMMING_MESSAGE_QUEUE_SIZE 10

/**
 * Outgoing message queue size
 */

#define GC_OUTGOING_MESSAGE_QUEUE_SIZE 10

#define GC_WRITE_ATTEMPTS 100

#define GC_NSEC_TIMEOUT 5000

/**
 * Number of NANOSECCONDS the logic thread will sleep between iterations.
 */
#define GC_LOGIC_THREAD_SLEEP GC_NSEC_TIMEOUT

/**
 * Number of attempts the logic thread will try to obtain a mutex.  Between tries it will sleep for a random number of nanoseconds.
 */
#define GC_MUTEX_LOCK_ATTEMPT 200

/**
 * Sets the size of the LOGIC_STATUS digital input buffer.  Obviously this has to equal to or be greater than the actual number of digital inputs used.
 */
#define GC_LOGIC_DIGITAL_INPUTS 10

/**
 * Sets the size of the LOGIC_STATUS digital output buffer.
 * \see GC_LOGIC_DIGITAL_INPUTS
 */
#define GC_LOGIC_DIGITAL_OUTPUTS 10

/**
 * Sets the size of the LOGIC_STATUS analog input buffer.
 * \see GC_LOGIC_DIGITAL_INPUTS
 */
#define GC_LOGIC_ANALOG_INPUTS 10

/**
 * Sets the size of the LOGIC_STATUS analog output buffer.
 * \see GC_LOGIC_DIGITAL_INPUTS
 */
#define GC_LOGIC_ANALOG_OUTPUTS 10

/**
 * How long, in seconds, the watchdog thread will sleep between iterations
 */
#define GC_WATCHDOG_SLEEP_NSEC 500000000

/**
 * How many iterations the watchdog thread will wait before killing the process.
 * Total amount of time a thread is given to reset the watchdog is GC_WATCHDOG_SLEEP_SEC * GC_WATCHDOG_ATTEMPTS
 */
#define GC_WATCHDOG_ATTEMPTS (1000000000/GC_WATCHDOG_SLEEP_NSEC) * 4

/**
 * Size of the incoming serial data buffer.
 * \see BBB_HVAC::IOCOMM::SER_IO_COMM::buffer
 */
#define GC_SERIAL_BUFF_SIZE 256

/**
 * Number of lines in the processed line table.  Each line is of size GC_SERIAL_BUFF_SIZE
 * \see BBB_HVAC::IOCOMM::SER_IO_COMM::table_a
 * \see BBB_HVAC::IOCOMM::SER_IO_COMM::table_b
 *
 */
#define GC_SERIAL_LINE_TABLE_ENTRIES 128

/**
 * Baud rate of the serial port.  The value is the appropriate define for termios struct.
 */
#define GC_SERIAL_BAUD_RATE B115200

/**
 * Serial port to use for communication with the board.
 */
#define GC_SERIAL_PORT_A "ttyS0"

#define GC_SERIAL_PORT_B "ttyS4"

/**
 * Number of nanoseconds to sleep between iterations of the main serial processing thread.
 * We need some delay there to give other threads waiting to call into the serial thread a chance to grab the lock.
 * 10 useconds seems to be a workable value.
 * \see BBB_HVAC::IOCOMM::SER_IO_COMM::main_event_loop
 */
#define GC_SERIAL_THREAD_SLEEP 10000

/**
 * The depth of the local IO state cache.
 */
#define GC_IO_STATE_BUFFER_DEPTH 8

/**
 * Number of analog inputs on the board.
 * \xxx really need to centralize this information.  The logic thread also uses this info.
 */
#define GC_IO_AI_COUNT 8

/**
 * Number of digital outputs on the board.
 * \xxx really need to centralize this information.  The logic thread also uses this info.
 */
#define GC_IO_DO_COUNT 4

#endif /* CONFIG_H_ */
