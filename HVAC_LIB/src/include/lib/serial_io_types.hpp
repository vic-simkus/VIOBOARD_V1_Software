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

#ifndef SRC_INCLUDE_LIB_SERIAL_IO_TYPES_HPP_
#define SRC_INCLUDE_LIB_SERIAL_IO_TYPES_HPP_

#include <time.h>

#include <memory>
#include <utility>
#include <vector>
#include <queue>
#include <exception>

#include <locale>
#include <cstdint>

#include "lib/config.hpp"
#include "lib/tprotect_base.hpp"
#include "lib/exceptions.hpp"
#include "lib/string_lib.hpp"

#include <string.h>


/**
 \file Various types used by the serial comms stuff.
 */
namespace BBB_HVAC
{
	using namespace EXCEPTIONS;

	namespace IOCOMM
	{

		/**
		 * Base class for state cache instances.
		 */
		class CACHE_ENTRY_BASE
		{
		public:

			/**
			 * Constructor.
			 */
			CACHE_ENTRY_BASE();

			/**
			 * Copy constructor.
			 * \param _src Source of the copy.
			 */
			CACHE_ENTRY_BASE( const CACHE_ENTRY_BASE& _src );

			/**
			 * Destructor.
			 */
			virtual ~CACHE_ENTRY_BASE();

			/**
			 * Constructor that initializes the instance from the string representation.
			 * To put it in Java terms - deserialises an instance.
			 * @param _source String representation of the object.
			 */
			explicit CACHE_ENTRY_BASE( const std::string& _source );

			/**
			 * Converts the instance to a human readable string.  Used for debugging purposes and for serializing to the shims.
			 */
			std::string to_string( void ) const;

			/**
			 * Provides a mechanism for the derived class to provide its value in string form.  Used for serializing and deserializing.
			 * @return
			 */
			virtual std::string value_to_string( void ) const = 0;

			/**
			 * Provides a mechanism for the derived class to populate its value from a string representation.
			 * This method is invoked by from_string with a cleaned up value portion of the serialized string representation.
			 * \param _str String value
			 */
			virtual void value_from_string( const std::string& _str ) = 0;

		protected:
			/**
			 * Timestamp of the instantiation of the class.  Initialized in the constructor.
			 */
			timespec time_spec;

			/**
			 * Bit of a hack to get around the fact that you can't call pure virtual functions from a constructor.
			 * Derived classes should call this from from their string constructor to complete the deserialization process.
			 */
			void from_string( const std::string& _source );

		};

		/**
		 * \brief Base class for a cache entry that contains a 16 bit unsigned value.
		 */
		class CACHE_ENTRY_16BIT : public CACHE_ENTRY_BASE
		{
		public:
			/**
			 * Constructor.
			 */
			CACHE_ENTRY_16BIT();

			/**
			 * Constructor.
			 * @param _val Value of the entry.
			 */
			CACHE_ENTRY_16BIT( uint16_t _val );

			/**
			 * Constructor.
			 * \param _source String representation of an instance.  The class will be desrialized from the string representation.
			 */
			CACHE_ENTRY_16BIT( const std::string& _source );

			/**
			 * Destructor.
			 */
			virtual ~CACHE_ENTRY_16BIT();

			/**
			 * Returns the instances value.
			 * \return Instances value.
			 */
			uint16_t get_value( void ) const;

			/**
			 * Converts the instances value to a string.  Used for serialization.
			 * @return Instances value as a string.
			 */
			virtual std::string value_to_string( void ) const;

			/**
			 * Converts the supplied string to classes value.
			 * @param _str String representation of the instance.  Used for deserialization.
			 */
			virtual void value_from_string( const std::string& _str );

		protected:
			/**
			 * Instances value.
			 */
			uint16_t value;
		};

		/**
		 * \brief Base class for a cache entry that contains an 8 bit unsigned value.
		 */
		class CACHE_ENTRY_8BIT : public CACHE_ENTRY_BASE
		{
		public:
			/**
			 * Constructor.
			 */
			CACHE_ENTRY_8BIT();

			/**
			 * Constructor.
			 * @param _val Instances value.
			 */
			CACHE_ENTRY_8BIT( uint8_t _val );

			/**
			 * Constructor.
			 * @param _source String representation of the class.  Used in deserialization.
			 */
			CACHE_ENTRY_8BIT( const std::string& _source );

			/**
			 * Destructor.
			 */
			virtual ~CACHE_ENTRY_8BIT();

			/**
			 * Gets the value associated with the instance.
			 * @return Instances value.
			 */
			uint8_t get_value( void ) const;

			/**
			 * Converts the instances value to a string.
			 * @return String representation of the value.
			 */
			virtual std::string value_to_string( void ) const;

			/**
			 * Converts a string into a value suitable for the instance.
			 * @param _str String representation of the value.
			 */
			virtual void value_from_string( const std::string& _str );

		protected:

			/**
			 * Instance value.
			 */
			uint8_t value;
		};

		/**
		 * A wrapper around CACHE_ENTRY_8BIT.  No additional functionality.
		 * An instance represents the state of digital outputs on a board.  The value is raw binary data from the board.
		 * \see CACHE_ENTRY_8BIT
		 */
		class DO_CACHE_ENTRY : public CACHE_ENTRY_8BIT
		{
		public:
			DO_CACHE_ENTRY();
			DO_CACHE_ENTRY( uint8_t _value );
			DO_CACHE_ENTRY( const std::string& _source );
		};

		/**
		 * A wrapper around CACHE_ENTRY_8BIT.  No additional functionality.
		 * An instance represents the state of a PMIC circuit on a board.  The value is raw binary data from the board.
		 * \see CACHE_ENTRY_8BIT
		 */
		class PMIC_CACHE_ENTRY : public CACHE_ENTRY_8BIT
		{
		public:
			PMIC_CACHE_ENTRY();
			PMIC_CACHE_ENTRY( uint8_t _value );
			PMIC_CACHE_ENTRY( const std::string& _source );
		};

		/*
		 * ADC (analog digital converter) value cache entry.  The IO thread automatically sucks down ADC values from the associated IO board.
		 * The values are stored in instances of this class.  Reason for not using just a POD value is that we want to know when that value was read so we can eliminate duplicates.
		 */
		class ADC_CACHE_ENTRY : public CACHE_ENTRY_16BIT
		{
		public:

			/**
			 * Constructor.  Exists solely to make the compiler happy.
			 */
			ADC_CACHE_ENTRY();

			/**
			 * Instantiates an object from the supplied string representation.
			 * The format is expected to be: [Xsec.Xnsec:Val]
			 * @param _source
			 */
			explicit ADC_CACHE_ENTRY( const std::string& _source );

			/**
			 * The real constructor.  Initializes the value to the supplied parameter.
			 * \param _val ADC value
			 */
			ADC_CACHE_ENTRY( uint16_t _val );
		};

		/**
		A calibration value cache entry.
		*/
		class CAL_VALUE_ENTRY : public CACHE_ENTRY_16BIT
		{
		public:
			CAL_VALUE_ENTRY();
			CAL_VALUE_ENTRY( uint16_t _val );
			explicit CAL_VALUE_ENTRY( const std::string& _source );
		};


		/**
		 * Outgoing message.
		 * Outgoing messages are not sent immediately.  They are queued and written out by a separate thread.
		 */
		class OUTGOING_MESSAGE
		{
		public:

			OUTGOING_MESSAGE();
			OUTGOING_MESSAGE( const unsigned char* _buffer, const size_t _length );
			OUTGOING_MESSAGE( const OUTGOING_MESSAGE& _src );
			~OUTGOING_MESSAGE();

			/**
			 * Data buffer.
			 */
			std::shared_ptr<unsigned char> message;

			/**
			 * Length of the data buffer.
			 */
			size_t message_length;

			/**
			 * Has the message been sent yet.  Set to true when the message is submitted for transmittal and cleared once the message is sent.
			 */
			bool is_new;

			/**
			 * Message ID.  A sequential number that is incremented with each message.
			 */
			unsigned long id;
		};

		/**
		 * Errors that the IO class/thread can produce.
		 */
		enum class ENUM_ERRORS
			: unsigned int
		{
			ERR_NONE = 0, /// No error
			ERR_FAIL_LOCK, /// Failed to obtain lock.
			ERR_PORT_OPEN, /// Failed to open port device.  In case of serial board, the serial port.
			ERR_COMM, /// General communications failure.
			ERR_ATTRIBUTES /// Failure in setting port attributes.
		};

		/**
		 * \brief A table that contains messages received from the board.
		 * At this stage the messages have been individually identified from the incoming stream, but they have yet to be processed.
		 */
		class LINE_TABLE
		{
		public:

			/**
			 * \brief Constructor.
			 */
			inline LINE_TABLE() {
				this->table = ( unsigned char** ) malloc( sizeof( unsigned char* ) * GC_SERIAL_LINE_TABLE_ENTRIES );

				for( int i = 0; i < GC_SERIAL_LINE_TABLE_ENTRIES; i++ ) {
					this->table[i] = ( unsigned char* ) malloc( GC_SERIAL_BUFF_SIZE * sizeof( unsigned char ) );
					memset( this->table[i], 0xFF, GC_SERIAL_BUFF_SIZE * sizeof( unsigned char ) );
				}

				this->index = 0;
			}

			/**
			 * \brief Destructor.
			 */
			inline ~LINE_TABLE() {
				for( int i = 0; i < GC_SERIAL_LINE_TABLE_ENTRIES; i++ ) {
					free( this->table[i] );
					this->table[i] = nullptr;
				}

				free( this->table );
				this->table = nullptr;
			}

			/**
			 * Table entries.
			 */
			unsigned char** table;

			/**
			 * Index of the current entry.
			 * XXX - management of this is ... interesting at best.
			 */
			size_t index;
		};

		/**
		 * \brief Current status of the serial holding buffer.  All the raw data that is received from the board is dumped into this buffer.
		 * The idea is to drain the serial link as quickly as possible so as to avoid possibility of data loss.
		 * XXX - on modern processors, where even the BBB is running in the GHz range, does this even matter?
		 */
		typedef struct
		{
			/**
			 * Current position within the buffer.  It is incremented every time a byte is read from the serial port and added to the read buffer.
			 * It is the upper bound of how much data is in the buffer.
			 */
			size_t buffer_idx;

			/**
			 * Index within the buffer where assemble_serial_data is currently at.  It should always be less than buffer_index;
			 * Within the scope of the context rather than the method so that the method can stop and resume assembling the data as it becomes available.
			 * \see assemble_serial_data(void)
			 */
			size_t buffer_work_index;

			/**
			 * Used to skip past any leading line terminators.
			 * If the first character in the buffer is a line terminator, buffer_start_index will be 1 so that we ignore the said line terminator.
			 */
			size_t buffer_start_index;
			size_t bin_msg_start_index;
			bool in_bin_message;

		} ST_SERIAL_BUFFER_CONTEXT;

		/**
		 * Protocol (P) message parsing token.  Contains start and end indexes of a token.
		 * Its all part of my need to have both text and binary protocol on the same wire.
		 */
		typedef std::pair<size_t, size_t> token;

		/**
		 * A list of protocol message tokens.  Basically a line tokenized.
		 */
		typedef std::vector<token> token_vector;
	}
}

#include "lib/outgoing_message_queue.hpp"
#include "lib/board_state_cache.hpp"
#include "lib/serial_io_type_operators.hpp"

#endif /* SRC_INCLUDE_LIB_SERIAL_IO_TYPES_HPP_ */
