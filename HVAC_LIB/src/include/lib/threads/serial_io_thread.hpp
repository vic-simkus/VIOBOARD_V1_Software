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


#ifndef BBB_HVAC_IOCOMM_HPP_
#define BBB_HVAC_IOCOMM_HPP_

#include <string>
#include "lib/config.hpp"

using namespace std;

#include <termios.h>

#include "lib/logger.hpp"
#include "lib/serial_io_types.hpp"
#include "lib/exceptions.hpp"
#include "lib/threads/thread_base.hpp"

#include <stdint.h>

namespace BBB_HVAC
{
	/**
	 * Namespace for all IO communication related stuffs.  Like all of the other components of the system this component
	 * runs in it's own thread.
	 */
	namespace IOCOMM
	{

		/**
		 * Base class for classes providing communication facilities between the program and I/O board.
		 */
		class IO_COMM_BASE : public THREAD_BASE
		{
		public:

			/**
			 * Constructor
			 * \param _tag Name of the instance.  Used for debugging purposes.
			 */
			inline IO_COMM_BASE( const string& _tag ) :
				THREAD_BASE( _tag ) {
				this->tag = _tag;
				this->is_io_thread = true;
				return;
			}

			/**
			 * The main entry point for the thread.
			 * The thread function will enter this method on startup.  When this method returns the thread will end.
			 * \return
			 */
			virtual bool main_event_loop( void ) = 0;

			/**
			 *  The write sub-thread.  The main thread function will start the write sub-thread.
			 * \return
			 */
			virtual bool write_event_loop( void ) = 0;

			/**
			Returns the tag of this thread.  The tag is used for debugging, log output, etc.
			\return Thread's tag
			*/
			inline const string& get_tag( void ) const {
				return this->tag;
			}

		protected:
			/**
			 * Instance tag.  Used for log output for debugging.
			 */
			string tag;

		private:

		} ;

		/**
		 * Provides communication services between a RS-232 IO board and the world.
		 */
		class SER_IO_COMM : public IO_COMM_BASE
		{
			/**
			 * Shim function that's used to start a thread.
			 */
			friend void serial_io_shim_func( void* );

		public:

			/**
			Call indexes that the board supports.
			*/
			enum ENUM_BOARD_COMMANDS : unsigned char
			{
				CMD_ID_GET_AI_STATUS = 0x01,	//	0x01
				CMD_ID_GET_DO_STATUS,			//	0x02
				CMD_ID_GET_PMIC_STATUS,			//	0x03
				CMD_ID_GET_L1_CAL_VALS,			//	0x04
				CMD_ID_GET_L2_CAL_VALS,			//	0x05
				CMD_ID_GET_BOOT_COUNT,			// 	0x06
				CMD_ID_GET_BOARD_STATS,			//	0x07

				CMD_ID_SET_DO_STATUS,			//	0x08
				CMD_ID_SET_PMIC_STATUS,			//	0x09
				CMD_ID_SET_L1_CAL_VALS,			//	0x0A
				CMD_ID_SET_L2_CAL_VALS,			//	0x0B

				CMD_ID_GET_CONFIRM_OUTPUT,		//	0x0C
				CMD_ID_START_STREAM,			//	0x0D
				CMD_ID_RESET_BOARD,				//	0x0E

				CMD_ID_SYS_FAILURE = 0xFF
			} ;

			/**
			 * Constructor.  Instantiating the class DOES NOT initialize it.
			 * \see init(void)
			 * \param _tty  Device to communicate through.  Just the name.  No /dev prefix.  ttyS0, for example.  Not /dev/ttyS0
			 * \param _tag Tag destined for human consumption used for debugging purposes.
			 */
			SER_IO_COMM( const char* _tty, const string& _tag, bool _debug );

			/**
			 * Initializes the instance.  Must be called after instantiation.
			 * \return
			 */
			ENUM_ERRORS init( void );

			/**
			 * Main event loop entry for the thread function.
			 * \return Should never return under normal circumstances.  If it does something went horribly wrong.
			 */
			bool main_event_loop( void );

			/**
			 * Write event loop for the sub-thread.
			 * @return
			 */
			bool write_event_loop( void ) throw( LOCK_ERROR );

			/**
			 * Destructor.  Should not be invoked casually.
			 */
			virtual ~SER_IO_COMM();

			/**
			 * Copies the cached DAC values into the supplied buffer.
			 * \param _dest Reference to the buffer to which to copy the values.  Why a highfalutin reference rather than a pointer?  Because I couldn't figure out the syntax to passing a pointer to a two dimensional array.
			 */
			bool get_dac_cache( ADC_CACHE_ENTRY( & _dest ) [GC_IO_STATE_BUFFER_DEPTH][GC_IO_AI_COUNT] );

			/**
			 * Copies the cached DO values into the supplied buffer.
			 * \param _dest Reference to the buffer to which to copy the values.
			 */
			bool get_do_cache( DO_CACHE_ENTRY( & _dest ) [GC_IO_STATE_BUFFER_DEPTH] );

			/**
			 * Copies the cached PMIC values into the supplied buffer.
			 * \param _dest Reference to the buffer to which to copy the values.
			 */
			bool get_pmic_cache( PMIC_CACHE_ENTRY( & _dest ) [GC_IO_STATE_BUFFER_DEPTH] );

			/**
			 * Copies the latest ADC values to the supplied buffer.
			 * \param _dest Reference to the destination buffer.
			 */
			bool get_latest_adc_values( ADC_CACHE_ENTRY( & _dest ) [GC_IO_AI_COUNT] );

			/**
			 * Copies the latest DO statuses to the supplied buffer.
			 * \param _dest Reference to the destination buffer.
			 */
			bool get_latest_do_status( DO_CACHE_ENTRY& _dest );

			/**
			 * Copies the latest PMIC statuses to the supplied buffer.
			 * \param _dest Reference to the destination buffer.
			 */
			bool get_latest_pmic_status( PMIC_CACHE_ENTRY& _dest );

			/**
			 * Copies all of latest values into the supplied buffers.
			 */
			bool get_latest_state_values( BOARD_STATE_CACHE& _target );

			/**
			 * Creates and sends a command to the board to set the digital outputs to specified states.
			 * \param _status Status bits.  All outputs are set using one byte.
			 */
			bool cmd_set_do_status( uint8_t _status );

			/**
			 * Creates and sends a command to the board to set the PMICS to specified states.
			 * \param _status Status bits.  Both PMICs are modified using one byte.
			 */
			bool cmd_set_pmic_status( uint8_t _status );

			/**
			Sends L1 calibration values to the board.
			*/
			bool cmd_set_l1_calibration_values( const CAL_VALUE_ARRAY& _values );

			/**
			Sends L2 calibration values to the board.
			*/
			bool cmd_set_l2_calibration_values( const CAL_VALUE_ARRAY& _values );

		protected:

			/**
			Sends calibration values to the board.
			\param _cmd Command (L1 or L2) to utilize when sending to board.
			*/
			bool cmd_set_calibration_values( unsigned char _cmd, const CAL_VALUE_ARRAY& _values );

			/**
			 * Performs a hard reset of the attached board.
			 */
			bool cmd_reset_board( void );

			/**
			Puts the board in a 'push' configuration.  After this command is invoked and until the board is reset it will continually output
			its state in binary message format.
			*/
			bool cmd_start_stream( void );

			/**
			Confirms the output state of the board.
			\see Board communications document
			*/
			bool cmd_confirm_output_state( void );

			/**
			 * Returns the process ID parsed out of the provided file that is the process ID that holds the lock on a serial port.  Used by the instance during locking checks/attempts of the serial port during initialization.
			 * Should not be invoked casually.
			 * \param _file The file  that contains the process ID.  This is not the serial port.  This is the lock file that contains the PID.
			 * \return   ID of the process that supposedly holds the lock.
			 */
			string get_locking_pid( const string& _file );

			/**
			 * Returns the command line of the supplied process ID.  Used during the lock attempts/checks of the serial port during the initialization of the instance.  Should not be invoked casually.
			 * \param _pid Process ID
			 * \return  Command line of the provided process ID
			 */
			string get_locking_pid_command( const string& _pid );

			/**
			 * Ascertains the status of the provided process ID.
			 * \param _pid Process ID
			 * \return True if the process is alive.  False otherwise.
			 */
			bool is_process_alive( const string& _pid );

			/**
			 * Generates a lock file name for the supplied serial port.
			 * \param _tty Serial port name.
			 * \return Name of the lock file.
			 */
			string generate_lock_file_name( const char* _tty );

			/**
			 * Generates a full device file name for the supplied serial port.
			 * \param _tty  Serial port name sans the /dev preamble.
			 * \return  Full device file name.
			 */
			string generate_port_device_file_name( const char* _tty );

			/**
			 * Tries to lock the serial port that was specified during the instantiation of the class.
			 * \return True if the lock was obtained, false otherwise.
			 */
			bool try_lock_serial( void );

			/**
			 * Sends out all pending messages to the IO board
			 * \return True if everything went as expected, false if an error was encountered.
			 */
			bool write_buffer( const unsigned char* _buffer, size_t _length );

			/**
			 * Parses and assembles the data in the input buffer.  The method is invoked whenever there is a lull in the
			 * receipt of serial data i.e. when the poll call times out.
			 * \return True if everything went as expected, false if an error was encountered.
			 */
			int assemble_serial_data( void );

			/**
			 * Reads all of the available serial data from the port and shoves it into the holding buffer.
			 * \return True is everything was handled without drama, false otherwise.
			 */
			bool drain_serial( void );

			/**
			 * Sends a message to the IO board.  The message is expected to contain the terminating newline (\n).
			 * The invoker is expected to supply a valid message.  No sanity checking is done by the method.
			 * \param _message Buffer containing the message to send.
			 * \param  length Length of the data in the buffer to send
			 * \return True if the message contents were submitted to the send queue, false otherwise.
			 */
			bool send_message( const unsigned char* _message, size_t length );

			/**
			 * Compacts the incoming message line table to ensure that there are no gaps.
			 * As the lines are processed they are removed from the line table which leaves gaps.  This method removes the gaps and compacts the table.
			 * \return True if everything went as expected, false otherwise.
			 */
			bool digest_line_table( void );

			/**
			 * Swaps the active line tables around.  There are two line tables for each instance.  An active and a passive.  The passive contains data from the previously sent command.
			 * The active table contains (or is waiting for) results/output from the command most recently sent to the IO board.  The tables are switched around every time a message is
			 * sent out to the board.
			 */
			void swap_tables( void );

			/**
			 * Adds an analog input (ADC) result to the cache.
			 * \param _line_index Line index in the active line table from which to assemble the data.
			 * \return True if everything went as expected, false otherwise.
			 */
			bool add_ai_result( size_t _line_index );

			/**
			 * Adds a digital output status result to the cache.
			 * \param _line_index Line index in the active line table from which to assemble the data.
			 * \return True if everything went as expected, false otherwise.
			 */
			bool add_do_status( size_t _idx );

			/**
			 * Adds a PMIC status result to the cache.
			 * \param _line_index Line index in the active line table from which to assemble the data.
			 * \return True if everything went as expected, false otherwise.
			 */
			bool add_pmic_status( size_t _idx );

			/**
			Adds calibration values to the state cache.
			*/
			bool add_calibration_values( size_t _idx, unsigned char _level );

			/**
			Adds boot count to the state cache.
			*/
			bool add_boot_count( size_t _idx );



			/**
			 * Adds data to the line table.
			 * \param _buffer Source of the data
			 * \param _length Length of the data to be copied.
			 */
			void add_to_active_table( const unsigned char* _buffer, size_t _length );

			/**
			Compacts the line table by purging processed lines.
			*/
			void compact_line_table( void );

			/**
			Returns true if the specified line is blank.
			\param _idx index of the line to inquire about.
			*/
			bool is_line_table_line_blank( size_t _idx ) const;

			/**
			Returns the index of the next non-blank line.
			\param _start_index Starting point of the search.
			*/
			size_t find_next_nonblank_line_table_line( size_t _start_idx ) const;

			/**
			Swaps to table lines.
			*/
			bool swap_two_line_table_lines( size_t _target_idx, size_t _source_idx );

			/**
			Marks the current table line as blank.
			*/
			void clear_active_table_current_line( void );

			/**
			Marks the line at index as blank.
			\param _idx Index of the line to mark as blank.
			*/
			void set_active_table_line_blank( size_t _idx );

			/**
			Processes a line as a binary message.
			\param _idx Index of the line to process as a binary message.
			*/
			void process_binary_message( size_t _idx );

			/**
			Process a line as a text message.
			\param _idx Index of the line to process as a text message.
			*/
			void process_text_message( size_t _idx );

			/**
			Processes a line as a protocol text message.
			\param _idx Index of the line to process as a protocol message.
			*/
			void process_protocol_message( size_t _idx );

			/**
			Tokenizes a line.  The line is expected to be a protocol line.
			\param _idx Index of the line to tokenize.
			\return A vector of indexes of token starts in the protocol line.
			*/
			token_vector tokenize_protocol_line( size_t _idx );

			/**
			Turns a a vector of token indexes into a vector of string tokens.
			\param _vect Token index vector.
			\param _idx Line index to which to apply the token vector.
			*/
			vector<string> create_protocol_line_tokens( const token_vector& _vect, size_t _idx );

			/**
			Thread entry function for general operations.
			*/
			bool thread_func( void );

			/**
			Thread entry function for write operations
			*/
			void writer_thread_func( void );

			/**
			Closes the serial port associated with the IO thread.  Does not unlock the port.
			*/
			void serial_port_close( void );

			/**
			Opens the serial port associated with the IO thread.  Does try to lock the port and fails if it can not.
			*/
			ENUM_ERRORS serial_port_open( void );

			/**
			Handles a board that is deemed to be locked/wedged.
			*/
			void handle_hung_board( void );

			/**
			Determines if it's ok to send data board.  This should be filed under ugly hacks.
			It checks to see if outgoing serial port queue is empty and the CTS line is high.  If both of those conditions are true it is determined that it's OK to write data to port.
			\return 0 - not clear to send data.  1 - clear to send data.  >1 error condition.
			*/
			unsigned char clear_to_send( void ) const;
		private:

			/**
			 * A yet unsent outgoing message
			 */
			OUTGOING_MESSAGE_QUEUE* outgoing_messages;

			/**
			 * Temporary holding buffer.  Filled with incoming serial data and read by assemble_serial_data.
			 * Size is determined by GC_SERIAL_BUFF_SIZE.
			 * \see assemble_serial_data
			 */
			unsigned char* buffer;

			/**
			Serial buffer context.
			*/
			ST_SERIAL_BUFFER_CONTEXT buffer_context;

			/**
			Resets the serial buffer context.
			*/
			void reset_buffer_context( void );

			/**
			 * Line table.  Filled by assemble_serial_data with data parsed from buffer.
			 * \see assemble_serial_data
			 * \see buffer
			 */
			LINE_TABLE table_a;

			/**
			 * Alternate line table
			 * \see assemble_serial_data
			 * \see table_a
			 */
			LINE_TABLE table_b;

			/**
			 * The active line table.
			 * \see swap_tables(void);
			 */
			LINE_TABLE* active_table;

			/**
			 * The passive line table.
			 * \see swap_tables(void);
			 */
			LINE_TABLE* passive_table;

			/**
			 * Full path to the tty devices.
			 */
			string tty_dev;

			/**
			 * Full path to the lock file.
			 */
			string lock_file;

			/**
			 * Termios information for the serial port before we go mucking around with it.
			 */
			struct termios old_tio;

			/**
			 * Termios information we set for our nefarious uses.
			 */
			struct termios current_tio;

			/**
			 * File descriptor of the open serial port.
			 */
			int serial_fd;

			/**
			Board state cache.
			*/
			BOARD_STATE_CACHE state_cache;

			/**
			Has the board reset.  True if it has, false otherwise.  We use this to make sure that the board is in a known state.
			*/
			bool board_has_reset;

			/**
			Is this board in a debug state.  If it is the board is never reset so as not to screw up the hardware debugger.
			*/
			bool in_debug_mode;

			/**
			Has stream mode started.
			*/
			bool stream_started;
		} ;

		inline void serial_io_shim_func( void* _parm )
		{
			( ( SER_IO_COMM* ) _parm )->writer_thread_func();
			return;
		}

	}
}

#endif /* BBB_HVAC_IOCOMM_HPP_ */
