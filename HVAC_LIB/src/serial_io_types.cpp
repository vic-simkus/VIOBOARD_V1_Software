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

#include <cstring>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#include "lib/serial_io_types.hpp"
#include "lib/logger.hpp"
#include "lib/memory_management.hpp"

#define MAX_BUFF_SIZE 16

namespace BBB_HVAC
{
	namespace IOCOMM
	{

		/************************************************************************
		 *
		 * OUTGOING_MESSAGE_QUEUE
		 *
		 ************************************************************************/

		OUTGOING_MESSAGE_QUEUE::OUTGOING_MESSAGE_QUEUE( const std::string& _tag ) : TPROTECT_BASE( _tag )
		{
			INIT_LOGGER( "BBB_HVAC::IOCOMM::OUTGOING_MESSAGE_QUEUE[" + this->tag + "]" );

			if ( pthread_cond_init( &this->conditional, nullptr ) != 0 )
			{
				LOG_ERROR( "Failed to initializer conditional." );
			}

			this->id_seq = 0;
			return;
		}

		OUTGOING_MESSAGE_QUEUE::~OUTGOING_MESSAGE_QUEUE()
		{
			if ( pthread_cond_destroy( &this->conditional ) != 0 )
			{
				LOG_ERROR( "Failed to destroy conditional." );
			}

			while ( !this->message_queue.empty() )
			{
				this->message_queue.pop();
			}

			return;
		}

		void OUTGOING_MESSAGE_QUEUE::signal( void )
		{
			/*
			 * We're assuming that we hold the lock right now.
			 */
			if ( pthread_cond_signal( &this->conditional ) != 0 )
			{
				LOG_ERROR( "Failed to signal conditional." );
			}

			this->release_lock();
			return;
		}

		bool OUTGOING_MESSAGE_QUEUE::add_message( const OUTGOING_MESSAGE& _msg )
		{
			bool ret = true;
			this->obtain_lock_ex();

			if ( this->message_queue.size() >= GC_OUTGOING_MESSAGE_QUEUE_SIZE )
			{
				this->release_lock();
				ret = false;
				LOG_ERROR( "Outgoing message queue is full." );
			}
			else
			{
				OUTGOING_MESSAGE msg = _msg;
				msg.id = this->id_seq;
				this->id_seq += 1;
				this->message_queue.push( _msg );
				this->signal();
			}

			return ret;
		}

		bool OUTGOING_MESSAGE_QUEUE::has_more_messages( void ) const
		{
			return !this->message_queue.empty();
		}

		OUTGOING_MESSAGE OUTGOING_MESSAGE_QUEUE::get_message( void )
		{
			OUTGOING_MESSAGE ret = this->message_queue.front();
			this->message_queue.pop();
			return ret;
		}

		void OUTGOING_MESSAGE_QUEUE::get_lock( void ) throw( LOCK_ERROR )
		{
			this->obtain_lock_ex();
			return;
		}

		void OUTGOING_MESSAGE_QUEUE::put_lock( void ) throw( LOCK_ERROR )
		{
			if ( this->release_lock() == false )
			{
				THROW_EXCEPTION( LOCK_ERROR, "Failed to release lock.  Check logs." );
			}

			return;
		}

		bool OUTGOING_MESSAGE_QUEUE::wait_for_signal( void ) throw( LOCK_ERROR )
		{
			this->get_lock();
			timespec timeout_time;
			timeout_time.tv_nsec = 0;
			timeout_time.tv_sec = 0;
			timeout_time.tv_sec = time( nullptr ) + 2;
			int rc = pthread_cond_timedwait( & ( this->conditional ), & ( this->mutex ), &timeout_time );

			if ( rc != 0 )
			{
				if ( rc == ETIMEDOUT )
				{
					/*
					 * Apparently even when a wait for a conditional times out the mutex is still acquired by us.
					 */
					this->release_lock();
					return false;
				}
				else
				{
					THROW_EXCEPTION( LOCK_ERROR, "Failed to obtain lock for reasons other than timeout." );
				}
			}
			else
			{
				return true;
			}
		}

		/************************************************************************
		 *
		 * CACHE_ENTRY_BASE
		 *
		 ************************************************************************/

		CACHE_ENTRY_BASE::CACHE_ENTRY_BASE()
		{
			clock_gettime( CLOCK_MONOTONIC, &this->time_spec );
			return;
		}

		CACHE_ENTRY_BASE::CACHE_ENTRY_BASE( const CACHE_ENTRY_BASE& _src )
		{
			this->time_spec = _src.time_spec;
			return;
		}

		CACHE_ENTRY_BASE::~CACHE_ENTRY_BASE()
		{
			this->time_spec.tv_sec = 0;
			this->time_spec.tv_nsec = 0;
			return;
		}

		CACHE_ENTRY_BASE::CACHE_ENTRY_BASE( const std::string& _source )
		{
			/*
			 * Basically the data we will be getting supplied is in the format of:
			 * [00000.0000:value] where '0' is a number from 0 to 9 and value is some sub-class specific numerical value
			 *
			 * Below we start scanning the buffer.  The trailing ']' breaks the loop.
			 */
			unsigned char buffer_left[MAX_BUFF_SIZE];
			unsigned char buffer_right[MAX_BUFF_SIZE];
			memset( buffer_left, 0, MAX_BUFF_SIZE );
			memset( buffer_right, 0, MAX_BUFF_SIZE );
			unsigned char* work_buffer = ( unsigned char* )( &buffer_left );
			const char* source_buffer = _source.data();
			size_t buffer_index = 0;

			for ( size_t i = 0; i < _source.length(); i++ )
			{
				if ( source_buffer[i] == '.' )
				{
					work_buffer = ( unsigned char* )( &buffer_right );
					buffer_index = 0;
				}
				else if ( source_buffer[i] >= 48 && source_buffer[i] <= 57 ) // 0 ... 9
				{
					work_buffer[buffer_index] = ( unsigned char )source_buffer[i];
					buffer_index += 1;

					if ( buffer_index == MAX_BUFF_SIZE )
					{
						/*
						 * Overflow.  Some dickhead passed bad data to us.
						 */
						return;
					}
				}
				else
				{
					/*
					 * A character is neither a number or a period.
					 * If we have found a first number already break out of the loop
					 */
					if ( buffer_index > 0 )
					{
						break;
					}
				}
			} //main loop

			this->time_spec.tv_sec = strtol( ( const char* ) buffer_left, nullptr, 10 );
			this->time_spec.tv_nsec = strtol( ( const char* ) buffer_right, nullptr, 10 );
			return;
		}

		std::string CACHE_ENTRY_BASE::to_string( void ) const
		{
			std::stringstream out;
			out << "[" << this->time_spec.tv_sec << "." << this->time_spec.tv_nsec << ":" << this->value_to_string() << "]";
			return out.str();
		}

		void CACHE_ENTRY_BASE::from_string( const std::string& _source )
		{
			size_t col_sep_idx = _source.find_last_of( ':' );
			size_t brac_sep_idx = _source.find_last_of( ']' );

			if ( brac_sep_idx <= col_sep_idx )
			{
				return;
			}

			this->value_from_string( _source.substr( col_sep_idx + 1, brac_sep_idx - ( col_sep_idx + 1 ) ) );
		}

		/************************************************************************
		 *
		 * CACHE_ENTRY_16BIT
		 *
		 ************************************************************************/

		CACHE_ENTRY_16BIT::CACHE_ENTRY_16BIT() : CACHE_ENTRY_BASE()
		{
			this->time_spec.tv_sec = 0;
			this->time_spec.tv_nsec = 0;
			this->value = 0;
			return;
		}

		CACHE_ENTRY_16BIT::~CACHE_ENTRY_16BIT()
		{
			this->value = UINT16_MAX;
			return;
		}

		CACHE_ENTRY_16BIT::CACHE_ENTRY_16BIT( uint16_t _val ) : CACHE_ENTRY_BASE()
		{
			this->value = _val;
			return;
		}

		CACHE_ENTRY_16BIT::CACHE_ENTRY_16BIT( const std::string& _source ) : CACHE_ENTRY_BASE( _source )
		{
			this->from_string( _source );
			return;
		}

		uint16_t CACHE_ENTRY_16BIT::get_value( void ) const
		{
			return this->value;
		}

		std::string CACHE_ENTRY_16BIT::value_to_string( void ) const
		{
			return num_to_str( this->value );
		}

		void CACHE_ENTRY_16BIT::value_from_string( const std::string& _str )
		{
			this->value = ( uint16_t ) stoul( _str );
		}

		/************************************************************************
		 *
		 * CACHE_ENTRY_8BIT
		 *
		 ************************************************************************/


		CACHE_ENTRY_8BIT::CACHE_ENTRY_8BIT() : CACHE_ENTRY_BASE()
		{
			this->time_spec.tv_sec = 0;
			this->time_spec.tv_nsec = 0;
			this->value = 0;
			return;
		}

		CACHE_ENTRY_8BIT::~CACHE_ENTRY_8BIT()
		{
			this->value = UINT8_MAX;
		}

		CACHE_ENTRY_8BIT::CACHE_ENTRY_8BIT( uint8_t _val ) : CACHE_ENTRY_BASE()
		{
			this->value = _val;
			return;
		}

		CACHE_ENTRY_8BIT::CACHE_ENTRY_8BIT( const std::string& _source ) : CACHE_ENTRY_BASE( _source )
		{
			this->from_string( _source );
			return;
		}

		uint8_t CACHE_ENTRY_8BIT::get_value( void ) const
		{
			return this->value;
		}

		std::string CACHE_ENTRY_8BIT::value_to_string( void ) const
		{
			return num_to_str( this->value );
		}

		void CACHE_ENTRY_8BIT::value_from_string( const std::string& _str )
		{
			this->value = ( uint8_t ) stoul( _str );
		}

		/************************************************************************
		 *
		 * ADC_CACHE_ENTRY
		 *
		 ************************************************************************/

		ADC_CACHE_ENTRY::ADC_CACHE_ENTRY() : CACHE_ENTRY_16BIT()
		{
			return;
		}

		ADC_CACHE_ENTRY::ADC_CACHE_ENTRY( const std::string& _source ) : CACHE_ENTRY_16BIT( _source )
		{
			return;
		}

		ADC_CACHE_ENTRY::ADC_CACHE_ENTRY( uint16_t _val ) : CACHE_ENTRY_16BIT( _val )
		{
			return;
		}

		/************************************************************************
		 *
		 * DO_CACHE_ENTRY
		 *
		 ************************************************************************/

		DO_CACHE_ENTRY::DO_CACHE_ENTRY() : CACHE_ENTRY_8BIT()
		{
			return;
		}

		DO_CACHE_ENTRY::DO_CACHE_ENTRY( uint8_t _value ) : CACHE_ENTRY_8BIT( _value )
		{
			return;
		}

		DO_CACHE_ENTRY::DO_CACHE_ENTRY( const std::string& _source ) : CACHE_ENTRY_8BIT( _source )
		{
			return;
		}

		/************************************************************************
		 *
		 * PMIC_CACHE_ENTRY
		 *
		 ************************************************************************/


		PMIC_CACHE_ENTRY::PMIC_CACHE_ENTRY() : CACHE_ENTRY_8BIT()
		{
			return;
		}

		PMIC_CACHE_ENTRY::PMIC_CACHE_ENTRY( uint8_t _value ) : CACHE_ENTRY_8BIT( _value )
		{
			return;
		}

		PMIC_CACHE_ENTRY::PMIC_CACHE_ENTRY( const std::string& _source ) : CACHE_ENTRY_8BIT( _source )
		{
			return;
		}

		/************************************************************************
		 *
		 * CAL_VALUE_ENTRY
		 *
		 ************************************************************************/

		CAL_VALUE_ENTRY::CAL_VALUE_ENTRY() : CACHE_ENTRY_16BIT()
		{
			return;
		}
		CAL_VALUE_ENTRY::CAL_VALUE_ENTRY( uint16_t _val ) : CACHE_ENTRY_16BIT( _val )
		{
			return;
		}
		CAL_VALUE_ENTRY::CAL_VALUE_ENTRY( const std::string& _source ) : CACHE_ENTRY_16BIT( _source )
		{
			return;
		}



		/************************************************************************
		 *
		 * OUTGOING_MESSAGE
		 *
		 ************************************************************************/

		OUTGOING_MESSAGE::OUTGOING_MESSAGE()
		{
			this->message.reset();
			this->message_length = 0;
			this->is_new = false;
			this->id = 0;
			return;
		}

		OUTGOING_MESSAGE::OUTGOING_MESSAGE( const unsigned char* _buffer, const size_t _length )
		{
			/*
			 * This is fucking brain dead
			 */
			/*
			this->message.reset( new unsigned char [_length], []( unsigned char * p )
			{
				delete [] p;
			} );

			 */
			this->message.reset( new unsigned char [_length], deleter_char_array );
			this->message_length = _length;
			memcpy( this->message.get(), _buffer, _length );
			this->is_new = true;
			return;
		}

		OUTGOING_MESSAGE::OUTGOING_MESSAGE( const OUTGOING_MESSAGE& _src )
		{
			this->message = _src.message;
			this->is_new = _src.is_new;
			this->id = _src.id;
			this->message_length = _src.message_length;
			return;
		}

		OUTGOING_MESSAGE::~OUTGOING_MESSAGE()
		{
			this->message.reset();
			this->message_length = 0;
			this->is_new = false;
			this->id = 0;
			return;
		}
	}
}
