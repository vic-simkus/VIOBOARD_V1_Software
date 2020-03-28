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
Default PID file name
*/
#define GC_PID_FILE "/tmp/bbb_hvac.pid"

#define GC_DEFAULT_TCPIP_PORT 6666
#define GC_DEFAULT_LISTEN_INTERFACE "127.0.0.1"

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
#define GC_INCOMMING_MESSAGE_QUEUE_SIZE 32

/**
 * Outgoing message queue size
 */

#define GC_OUTGOING_MESSAGE_QUEUE_SIZE 32

#define GC_WRITE_ATTEMPTS 100

#define GC_NSEC_TIMEOUT 5000

/**
 * Number of NANOSECCONDS the logic thread will sleep between iterations.
 */
#define GC_LOGIC_THREAD_SLEEP 999000000

/**
 * Number of logic loop iterations between saving of configuration
*/
#define GC_LOGIC_CONFIG_SAVE_INTERVAL	10

/**
 * Number of attempts the logic thread will try to obtain a mutex.  Between tries it will sleep for a random number of nanoseconds.
 */
#define GC_MUTEX_LOCK_ATTEMPT 400

/**
 * Sets the size of the LOGIC_STATUS digital output buffer.
 * \see GC_LOGIC_DIGITAL_INPUTS
 */
#define GC_LOGIC_DIGITAL_OUTPUTS 8

/**
 * Sets the size of the LOGIC_STATUS analog input buffer.
 * \see GC_LOGIC_DIGITAL_INPUTS
 */
#define GC_LOGIC_ANALOG_INPUTS 16

#define GC_LOGIC_SET_POINTS	32

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
#define GC_SERIAL_BUFF_SIZE 1024

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
//#define GC_SERIAL_BAUD_RATE B115200
#define GC_SERIAL_BAUD_RATE B19200


/**
 * Serial port to use for communication with the board.
 */
#define GC_SERIAL_PORT_A "ttyS0"

#define GC_SERIAL_PORT_B "ttyS4"

/**
 * Number of nanoseconds to sleep between iterations of the main serial processing thread.
 * We need some delay there to give other threads waiting to call into the serial thread a chance to grab the lock.
 * 10 useconds seems to be a workable value.
 \note The unit here is NANOSECONDS
 * \see BBB_HVAC::IOCOMM::SER_IO_COMM::main_event_loop
 */
#define GC_SERIAL_THREAD_SLEEP 10000

/**
Number of milliseconds before a poll call times out.
\note The unit here is MILLISECONDS
\see BBB_HVAC::IOCOMM::SER_IO_COMM::main_event_loop
*/
#define GC_SERIAL_THREAD_POLL_TIMEOUT 2
/**
Number of main serial thread processing loop iterations between pulls of status data from the IO board.
The nominal minimum interval between status updates from the board is (GC_SERIAL_THREAD_SLEEP + GC_SERIAL_THREAD_POLL_TIMEOUT) * GC_SERIAL_THREAD_UPADTE_INTERVAL.
The interval is actually longer since the nominal interval does not take into account all of the other processing that the loop does
XXX - need to update to reflect changes in timing mechanism
\see BBB_HVAC::IOCOMM::SER_IO_COMM::main_event_loop
*/
#define GC_SERIAL_THREAD_UPDATE_INTERVAL 250000
/**
 * The depth of the local IO state cache.
 */
#define GC_IO_STATE_BUFFER_DEPTH 1

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


#define GC_IO_ADC_VREF_MAX ((double)5.0)
#define GC_IO_ADC_STEPS	(4096)

#define GC_IO_ADC_STEP ((float)((float)GC_IO_ADC_VREF_MAX / (float)GC_IO_ADC_STEPS))

#define GC_PMIC_AI_EN_MASK 0x01
#define GC_PMIC_DO_EN_MASK 0x02
#define GC_PMIC_AI_ERR_MASK 0x04
#define GC_PMIC_DO_ERR_MASK 0x08

#define GC_DO_4_MASK 0x08
#define GC_DO_3_MASK 0x04
#define GC_DO_2_MASK 0x02
#define GC_DO_1_MASK 0x01

#define GC_PMIC_AI_EN_MASK 0x01
#define GC_PMIC_DO_EN_MASK 0x02
#define GC_PMIC_AI_ERR_MASK 0x04
#define GC_PMIC_DO_ERR_MASK 0x08

#endif /* CONFIG_H_ */
