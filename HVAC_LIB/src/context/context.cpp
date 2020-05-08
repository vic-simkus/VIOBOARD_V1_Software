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

#include "lib/config.hpp"
#include "lib/message_processor.hpp"
#include "lib/socket_reader.hpp"
#include "lib/logger.hpp"
#include "lib/string_lib.hpp"
#include "lib/globals.hpp"
#include "lib/threads/logic_thread.hpp"
#include "lib/hvac_types.hpp"

#include "lib/threads/serial_io_thread.hpp"
#include "lib/threads/thread_registry.hpp"

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

/*************************************
 *
 * Begin HS_CLIENT_CONTEXT stuff
 *
 *************************************/

HS_CLIENT_CONTEXT::HS_CLIENT_CONTEXT( int _client_socket ) :
	BASE_CONTEXT( "HS_CLIENT_CONTEXT", SOCKET_TYPE::NONE, "", 0 )
{
	//INIT_LOGGER( "BBB_HVAC::HS_CLIENT_CONTEXT" );
	this->remote_socket = _client_socket;
	LOG_DEBUG( "Created new HS_CLIENT_CONTEXT" );
	return;
}

HS_CLIENT_CONTEXT::~HS_CLIENT_CONTEXT()
{
	return;
}

ENUM_MESSAGE_CALLBACK_RESULT HS_CLIENT_CONTEXT::process_message( ENUM_MESSAGE_DIRECTION _direction, BASE_CONTEXT* _ctx, const MESSAGE_PTR& _message ) throw( exception )
{
	ENUM_MESSAGE_CALLBACK_RESULT ret = ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;

	if ( BASE_CONTEXT::process_message( _direction, _ctx, _message ) == ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED )
	{
		ret = ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
	}
	else
	{
		ENUM_MESSAGE_TYPE t = _message->get_message_type()->type;

		if ( t == ENUM_MESSAGE_TYPE::GET_LABELS )
		{
			MESSAGE_PTR m = this->message_processor->create_get_labels_message_response( CONFIG_ENTRY::string_to_type( _message->get_part_as_s( 0 ) ) );
			this->message_processor->send_message( m, this->remote_socket );

			ret = ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
		}
		else if ( t == ENUM_MESSAGE_TYPE::READ_STATUS_RAW_ANALOG )
		{
			std::string board_tag = _message->get_part_as_s( 0 );
			IOCOMM::SER_IO_COMM* comm_thread = THREAD_REGISTRY::get_serial_io_thread( board_tag );
			IOCOMM::ADC_CACHE_ENTRY dac_cache[GC_IO_STATE_BUFFER_DEPTH][GC_IO_AI_COUNT];
			comm_thread->get_dac_cache( dac_cache );
			vector<string> parts;

			for ( unsigned int i = 0; i < GC_IO_STATE_BUFFER_DEPTH; i++ )
			{
				for ( unsigned int j = 0; j < GC_IO_AI_COUNT; j++ )
				{
					parts.push_back( dac_cache[i][j].to_string() );
				}
			}

			MESSAGE_PTR m( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::READ_STATUS_RAW_ANALOG ), parts ) );
			this->message_processor->send_message( m, this->remote_socket );

			ret = ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
		}
		else if ( t == ENUM_MESSAGE_TYPE::READ_STATUS )
		{
			IOCOMM::DO_CACHE_ENTRY do_cache;
			IOCOMM::PMIC_CACHE_ENTRY pmic_cache;
			IOCOMM::ADC_CACHE_ENTRY dac_cache[GC_IO_AI_COUNT];
			IOCOMM::CAL_VALUE_ENTRY l1_cal_cache[GC_IO_AI_COUNT];
			IOCOMM::CAL_VALUE_ENTRY l2_cal_cache[GC_IO_AI_COUNT];
			std::string board_tag = _message->get_part_as_s( 0 );
			IOCOMM::SER_IO_COMM* comm_thread = THREAD_REGISTRY::get_serial_io_thread( board_tag );

			/*
			Rather than continually call into the serial IO thread and grabbing the lock we get the whole cache at once and tease out the individual components on our own time.
			*/
			IOCOMM::BOARD_STATE_CACHE state_cache( board_tag + "[t]" );
			comm_thread->get_latest_state_values( state_cache );
			state_cache.get_latest_adc_values( dac_cache );
			state_cache.get_latest_do_status( do_cache );
			state_cache.get_latest_pmic_status( pmic_cache );
			state_cache.get_latest_l1_cal_values( l1_cal_cache );
			state_cache.get_latest_l2_cal_values( l2_cal_cache );
			/*
			Response message parts
			*/
			vector<string> parts;

			/*
			Put the ADC values into the response.
			*/
			for ( size_t j = 0; j < GC_IO_AI_COUNT; j++ )
			{
				parts.push_back( dac_cache[j].to_string() );
			}

			/*
			Put the DO and PMIC status into the response.
			We do them kind of weird in the middle of arrays in order to maintain backwards compatibility with existing stuffs.
			Not that it matters since the other stuffs will be rewritten shortly.  Either way, it doesn't really matter.
			*/
			parts.push_back( do_cache.to_string() );
			parts.push_back( pmic_cache.to_string() );

			/*
			Put the L1 cal values into the response.
			*/
			for ( size_t j = 0; j < GC_IO_AI_COUNT; j++ )
			{
				parts.push_back( l1_cal_cache[j].to_string() );
			}

			/*
			Put the L2 cal values into the response.
			*/
			for ( size_t j = 0; j < GC_IO_AI_COUNT; j++ )
			{
				parts.push_back( l2_cal_cache[j].to_string() );
			}

			/*
			Finally put out the boot count.
			We wrap it into a cache entry in order to keep the format consistent.
			*/
			parts.push_back( IOCOMM::CACHE_ENTRY_16BIT( state_cache.get_boot_count() ).to_string() );
			MESSAGE_PTR m( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::READ_STATUS ), parts ) );
			this->message_processor->send_message( m, this->remote_socket );

			ret = ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
		}
		else if ( t == ENUM_MESSAGE_TYPE::SET_PMIC_STATUS )
		{
			/*
			 * Set PMIC Status
			 */
			std::string board_tag = _message->get_part_as_s( 0 );
			IOCOMM::SER_IO_COMM* comm_thread = THREAD_REGISTRY::get_serial_io_thread( board_tag );
			comm_thread->cmd_set_pmic_status( ( uint8_t ) _message->get_part_as_ui( 1 ) );

			ret = ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
		}
		else if ( t == ENUM_MESSAGE_TYPE::SET_STATUS )
		{
			/*
			 * Set DO Status
			 */
			std::string board_tag = _message->get_part_as_s( 0 );
			IOCOMM::SER_IO_COMM* comm_thread = THREAD_REGISTRY::get_serial_io_thread( board_tag );
			comm_thread->cmd_set_do_status( ( uint8_t ) _message->get_part_as_ui( 1 ) );

			ret = ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
		}
		else if ( t == ENUM_MESSAGE_TYPE::ERROR )
		{
			/*
			Why is the client sending error messages to us.  Does it really thing we care.
			*/
			LOG_WARNING( "Client sent us an error message for some unknown reason.  Message: " + _message->to_string() );

			ret = ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
		}
		else if ( t == ENUM_MESSAGE_TYPE::SET_L1_CAL_VALS )
		{
			std::string board_tag = _message->get_part_as_s( 0 );
			IOCOMM::SER_IO_COMM* comm_thread = THREAD_REGISTRY::get_serial_io_thread( board_tag );
			CAL_VALUE_ARRAY cal_values;

			for ( unsigned int i = 1; i < _message->get_part_count(); i++ )
			{
				cal_values.push_back( _message->get_part_as_ui( i ) );
			}

			comm_thread->cmd_set_l1_calibration_values( cal_values );

			ret = ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
		}
		else if ( t == ENUM_MESSAGE_TYPE::SET_L2_CAL_VALS )
		{
			std::string board_tag = _message->get_part_as_s( 0 );
			IOCOMM::SER_IO_COMM* comm_thread = THREAD_REGISTRY::get_serial_io_thread( board_tag );
			CAL_VALUE_ARRAY cal_values;

			for ( unsigned int i = 1; i < _message->get_part_count(); i++ )
			{
				cal_values.push_back( _message->get_part_as_ui( i ) );
			}

			comm_thread->cmd_set_l2_calibration_values( cal_values );

			ret = ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
		}
		else if ( t == ENUM_MESSAGE_TYPE::FORCE_AI_VALUE )
		{
			std::string board_tag = _message->get_part_as_s( 0 );
			uint8_t input = ( uint8_t )_message->get_part_as_ui( 1 );
			uint16_t value = _message->get_part_as_ui( 2 );

			//LOG_DEBUG( "Forcing (" + board_tag + ") AI" + num_to_str( input ) + " to value " + num_to_str( value ) + " -- " + _message->to_string() );

			IOCOMM::SER_IO_COMM* comm_thread = THREAD_REGISTRY::get_serial_io_thread( board_tag );
			comm_thread->force_ai_value( input, value );

			ret = ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
		}
		else if ( t == ENUM_MESSAGE_TYPE::UNFORCE_AI_VALUE )
		{
			std::string board_tag = _message->get_part_as_s( 0 );
			uint8_t input = ( uint8_t )_message->get_part_as_ui( 1 );
			IOCOMM::SER_IO_COMM* comm_thread = THREAD_REGISTRY::get_serial_io_thread( board_tag );
			comm_thread->unforce_ai_value( input );

			ret = ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
		}
		else if ( t == ENUM_MESSAGE_TYPE::READ_LOGIC_STATUS )
		{
			if ( GLOBALS::logic_instance == nullptr )
			{
				LOG_ERROR( "Why is the logic thread instance null?" );
			}
			else
			{
				/*
				Response message parts
				*/
				vector<string> parts;
				std::map<std::string, LOGIC_POINT_STATUS> logic_status = GLOBALS::logic_instance->get_logic_status();

				for ( auto map_iterator = logic_status.begin(); map_iterator != logic_status.end(); ++map_iterator )
				{
					parts.push_back( map_iterator->first );

					if ( map_iterator->second.is_double_value )
					{
						parts.push_back( num_to_str( map_iterator->second.double_value ) );
					}
					else
					{
						parts.push_back( num_to_str( map_iterator->second.bool_value ) );
					}
				}

				MESSAGE_PTR m( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::READ_LOGIC_STATUS ), parts ) );
				this->message_processor->send_message( m, this->remote_socket );
			}

			ret = ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
		}
		else if ( t == ENUM_MESSAGE_TYPE::SET_SP )
		{
			if ( GLOBALS::logic_instance == nullptr )
			{
				LOG_ERROR( "Why is the logic thread instance null?" );
			}
			else
			{
				GLOBALS::logic_instance->set_sp_value( _message->get_part_as_s( 0 ), _message->get_part_as_d( 1 ) );
			}

			ret = ENUM_MESSAGE_CALLBACK_RESULT::PROCESSED;
		}
		else
		{
			ret = ENUM_MESSAGE_CALLBACK_RESULT::IGNORED;
		}
	}

	// We don't flip the cond variable because on the logic_core side nothing is waiting for it.
	return ret;
}

/*************************************
 *
 * Begin HS_SERVER_CLIENT stuff
 *
 *************************************/

HS_SERVER_CONTEXT::HS_SERVER_CONTEXT( SOCKET_TYPE _st, const string& _path, uint16_t _port ) :
	BASE_CONTEXT( "HS_SERVER_CONTEXT", _st, _path, _port )
{
	INIT_LOGGER( "BBB_HVAC::HS_SERVER_CONTEXT" );
	return;
}

HS_SERVER_CONTEXT::~HS_SERVER_CONTEXT()
{
	unlink( GC_LOCAL_COMMAND_SOCKET );
	return;
}

