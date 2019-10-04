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

#include "lib/message_processor.hpp"
#include "lib/exceptions.hpp"
#include "lib/config.hpp"
#include "lib/string_lib.hpp"
#include "lib/threads/logic_thread.hpp"
#include "lib/globals.hpp"

#include <string.h>
#include <unistd.h>

#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

using namespace BBB_HVAC;
using namespace BBB_HVAC::MSG_PROC;

unsigned int MESSAGE_PROCESSOR::MAX_SUPPORTED_PROTOCOL = 1;

MESSAGE_PROCESSOR::MESSAGE_PROCESSOR()
{
	INIT_LOGGER( "BBB_HVAC::MESSAGE_PROCESSOR" );
	this->incomming_message_queue = new MSG_PROC::MESSAGE_QUEUE( GC_INCOMMING_MESSAGE_QUEUE_SIZE );
	this->outgoing_message_queue = new MSG_PROC::MESSAGE_QUEUE( GC_OUTGOING_MESSAGE_QUEUE_SIZE );
	this->protocol_negotiated = false;
}

MESSAGE_PROCESSOR::~MESSAGE_PROCESSOR()
{
	delete this->incomming_message_queue;
	delete this->outgoing_message_queue;
	this->incomming_message_queue = nullptr;
	this->outgoing_message_queue = nullptr;
	this->protocol_negotiated = false;
}

void MESSAGE_PROCESSOR::send_message( MESSAGE_PTR& _msg, int _fd ) throw( exception )
{
	const string& payload = _msg->get_payload();
	unique_ptr<char[] > buffer( new char[payload.length()] );
	memset( buffer.get(), 0, payload.length() );
	strncpy( buffer.get(), payload.data(), payload.length() );
	ssize_t rc = 0;
	ssize_t bytes_written = 0;
	unsigned int attempts = 0;

	do
	{
		rc = write( _fd, buffer.get(), ( int ) payload.length() );

		if ( rc == -1 )
		{
			throw EXCEPTIONS::MESSAGE_ERROR( create_perror_string( "Failed to write to client socket:" ) );
		}

		bytes_written += rc;
		attempts += 1;

		if ( attempts > GC_WRITE_ATTEMPTS )
		{
			if ( ( size_t ) bytes_written != payload.length() )
			{
				throw EXCEPTIONS::MESSAGE_ERROR( "Failed to write complete message after " + num_to_str( GC_WRITE_ATTEMPTS ) + "." );
			}
		}
	}
	while ( ( size_t ) bytes_written < payload.length() );

	_msg->tag_sent();
	this->outgoing_message_queue->add_message( _msg, ENUM_APPEND_MODE::LOSE_OVERFLOW );
	return;
}

MESSAGE_PTR MESSAGE_PROCESSOR::parse_message( const std::string& _buffer ) throw( exception )
{
	size_t sep_idx = 0;
	sep_idx = _buffer.find_first_of( MESSAGE::sep_char );

	if ( sep_idx == string::npos )
	{
		throw ( EXCEPTIONS::PROTOCOL_ERROR( "Failed to find separator character in message buffer." ) );
	}

	string tstr = _buffer.substr( 0, sep_idx );
	unsigned int msg_length = -1;

	try
	{
		msg_length = stoi( tstr );
	}
	catch ( const exception& e )
	{
		throw ( EXCEPTIONS::PROTOCOL_ERROR( string( "Failed to convert [" + tstr + "] to a number:" + e.what() ) ) );
	}

	if ( msg_length != _buffer.length() )
	{
		throw ( EXCEPTIONS::PROTOCOL_ERROR( string( "Supplied length parameter [" + tstr + "] is not the length of the buffer [" + num_to_str( ( unsigned int ) _buffer.length() ) + "] [" + _buffer + "]" ) ) );
	}

	/*
	 *
	 * Basic integrity checks out of the way.
	 *
	 */
	vector<string> parts;
	size_t prev_idx = sep_idx + 1;
	bool b = false;

	while ( 1 )
	{
		sep_idx = _buffer.find( MESSAGE::sep_char, prev_idx );

		if ( sep_idx == string::npos )
		{
			sep_idx = _buffer.length() - 1;  //-1 to get rid of trailing new line.
			b = true;
		}

		if ( sep_idx - prev_idx > 0 )
		{
			parts.push_back( _buffer.substr( prev_idx, sep_idx - prev_idx ) );
		}

		if ( b )
		{
			break;
		}

		prev_idx = sep_idx + 1;
	}

	/*
	 * At this point we're expecting at least one element in the parts vector - the message type.
	 */

	if ( parts.size() < 1 )
	{
		throw ( EXCEPTIONS::PROTOCOL_ERROR( "Could not parse buffer into a valid message.  No message type specified." ) );
	}

	const string message_type = parts.front();
	/*
	 * We don't count the message type as a part
	 */
	parts.erase( parts.begin() );
	MESSAGE_TYPE mt = MESSAGE_TYPE_MAPPER::get_message_type_by_label( message_type );

	if ( mt.get() == nullptr )
	{
		throw EXCEPTIONS::PROTOCOL_ERROR( "Invalid message type: [" + message_type + "]" );
	}

	if ( mt->type == ENUM_MESSAGE_TYPE::HELLO )
	{
		/*
		 * The HELLO message should have two parts - VERSION|X
		 */
		if ( parts.size() != 2 )
		{
			throw EXCEPTIONS::PROTOCOL_ERROR( "Invalid number of parts for a HELLO message.  Expecting 1, received " + num_to_str( ( unsigned int ) parts.size() ) + "." );
		}
	}
	else if ( mt->type == ENUM_MESSAGE_TYPE::PING || mt->type == ENUM_MESSAGE_TYPE::PONG )
	{
		if ( parts.size() != 0 )
		{
			throw EXCEPTIONS::PROTOCOL_ERROR( "Invalid number of parts for a PING or PONG message.  Expecting 0, received " + num_to_str( ( unsigned int ) parts.size() ) + "." );
		}
	}
	else if ( mt->type == ENUM_MESSAGE_TYPE::GET_LABELS )
	{
		/*
		 * The GET_LABELS message should have at least two parts - XX|YY where XX is the type DO, DI, etc and YY is either RESP or REQ
		 */
		if ( parts.size() < 2 )
		{
			THROW_EXCEPTION( EXCEPTIONS::PROTOCOL_ERROR, "Invalid number of parts for a GET_LABEL messages.  Expecting >=2, received < 2" );
		}
	}
	/*
	All of the messages bellow require a board tag since we can have more than board attached to the system
	*/
	else if ( mt->type == ENUM_MESSAGE_TYPE::SET_STATUS )
	{
		if ( parts.size() != 2 )
		{
			THROW_EXCEPTION( EXCEPTIONS::PROTOCOL_ERROR, "Invalid number of parts for a SET_STATUS message.  Expecting 2, received: " + num_to_str( ( unsigned int ) parts.size() ) + "." );
		}
	}
	else if ( mt->type == ENUM_MESSAGE_TYPE::SET_PMIC_STATUS )
	{
		if ( parts.size() != 2 )
		{
			THROW_EXCEPTION( EXCEPTIONS::PROTOCOL_ERROR, "Invalid number of parts for a SET_PMIC_STATUS message.  Expecting 2, received: " + num_to_str( ( unsigned int ) parts.size() ) + "." );
		}
	}
	else if ( mt->type == ENUM_MESSAGE_TYPE::READ_STATUS )
	{
		if ( parts.size() < 1 )
		{
			THROW_EXCEPTION( EXCEPTIONS::PROTOCOL_ERROR, "Invalid number of parts for a READ_STATUS message.  Expecting >1, received: " + num_to_str( ( unsigned int ) parts.size() ) + "." );
		}
	}
	else if ( mt->type == ENUM_MESSAGE_TYPE::READ_STATUS_RAW_ANALOG )
	{
		if ( parts.size() < 1 )
		{
			THROW_EXCEPTION( EXCEPTIONS::PROTOCOL_ERROR, "Invalid number of parts for a READ_STATUS_RAW_ANALOG message.  Expecting >1, received: " + num_to_str( ( unsigned int ) parts.size() ) + "." );
		}
	}
	else if ( mt->type == ENUM_MESSAGE_TYPE::ERROR )
	{
		if ( parts.size() < 2 )
		{
			THROW_EXCEPTION( EXCEPTIONS::PROTOCOL_ERROR, "Invalid number of parts for an ERROR message.  Expecting >2, received: " + num_to_str( ( unsigned int ) parts.size() ) + "." );
		}
	}
	else if ( mt->type == ENUM_MESSAGE_TYPE::FORCE_AI_VALUE )
	{
		if ( parts.size() < 3 )
		{
			THROW_EXCEPTION( EXCEPTIONS::PROTOCOL_ERROR, "Invalid number of parts for an FORCE_IN_VALUE message.  Expecting >3, received: " + num_to_str( ( unsigned int ) parts.size() ) + "." );
		}
	}
	else if ( mt->type == ENUM_MESSAGE_TYPE::UNFORCE_AI_VALUE )
	{
		if ( parts.size() < 2 )
		{
			THROW_EXCEPTION( EXCEPTIONS::PROTOCOL_ERROR, "Invalid number of parts for an UNFORCE_IN_VALUE message.  Expecting >2, received: " + num_to_str( ( unsigned int ) parts.size() ) + "." );
		}
	}

	MESSAGE_PTR ret( new MESSAGE( mt, parts ) );
	ret->tag_received();
	this->incomming_message_queue->add_message( ret, ENUM_APPEND_MODE::LOSE_OVERFLOW );
	return ret;
}

void MESSAGE_PROCESSOR::process_hello_message( void ) throw( exception )
{
	if ( this->incomming_message_queue->get_message_count() > 1 )
	{
		/*
		 * The hello message is expected to be the first  message received.
		 */
		throw EXCEPTIONS::PROTOCOL_ERROR( "Protocol sequence error.  Expecting HELLO message to be the first message received." );
	}

	MESSAGE_PTR msg = this->incomming_message_queue->get_message( 0 );
	unsigned int requested_protocol;

	try
	{
		requested_protocol = msg->get_part_as_ui( 1 );
	}
	catch ( const exception& e )
	{
		throw EXCEPTIONS::PROTOCOL_ERROR( "Failed to get message part: " + string( e.what() ) );
	}

	if ( requested_protocol > MESSAGE_PROCESSOR::MAX_SUPPORTED_PROTOCOL )
	{
		throw EXCEPTIONS::PROTOCOL_ERROR( "Protocol error.  Requested protocol version is higher than what we support.  Requested: " + num_to_str( requested_protocol ) + ".  We support: " + num_to_str( MESSAGE_PROCESSOR::MAX_SUPPORTED_PROTOCOL ) + "." );
	}

	this->protocol_negotiated = true;
	return;
}

MESSAGE_PTR MESSAGE_PROCESSOR::create_get_labels_message_response( ENUM_CONFIG_TYPES _type ) throw( exception )
{
	vector<string> parts;
	//parts.push_back( CONFIG_ENTRY::type_to_string( _type ) );
	//parts.push_back( "RESP" );
	std::vector<std::string> labels;
	LOGIC_STATUS_FLUFF fluff;
	GLOBALS::logic_instance->get_logic_status_fluff( fluff );

	switch ( _type )
	{
		case ENUM_CONFIG_TYPES::AI:
			labels.resize( fluff.ai_labels.size() );
			std::transform( fluff.ai_labels.begin(), fluff.ai_labels.end(), labels.begin(), BOARD_POINT::to_string_static );
			break;

		case ENUM_CONFIG_TYPES::DO:
			labels.resize( fluff.do_labels.size() );
			std::transform( fluff.do_labels.begin(), fluff.do_labels.end(), labels.begin(), BOARD_POINT::to_string_static );
			break;

		case ENUM_CONFIG_TYPES::SP:
			labels.resize( fluff.sp_labels.size() );
			std::transform( fluff.sp_labels.begin(), fluff.sp_labels.end(), labels.begin(), SET_POINT::to_string_static );
			break;

		case ENUM_CONFIG_TYPES::MAP:
			for ( auto map_iterator = fluff.point_map.begin(); map_iterator != fluff.point_map.end(); ++map_iterator )
			{
				labels.push_back( map_iterator->first );
				labels.push_back( map_iterator->second.to_string() );
			}

			break;

		default:
			THROW_EXCEPTION( runtime_error, "Can not respond to type specified: " + CONFIG_ENTRY::type_to_string( _type ) );
	}

	for ( std::vector<std::string>::const_iterator i = labels.cbegin(); i != labels.cend(); ++i )
	{
		parts.push_back( *i );
	}

	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::GET_LABELS ), parts ) );
}

MESSAGE_PTR MESSAGE_PROCESSOR::create_get_labels_message_request( ENUM_CONFIG_TYPES _type ) throw( exception )
{
	vector<string> parts;
	parts.push_back( CONFIG_ENTRY::type_to_string( _type ) );
	parts.push_back( "REQ" );
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::GET_LABELS ), parts ) );
}

MESSAGE_PTR MESSAGE_PROCESSOR::create_set_pmic_status( const std::string& _board_tag, uint8_t _status ) throw( exception )
{
	vector<string> parts;
	parts.push_back( _board_tag );
	parts.push_back( num_to_str( _status ) );
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::SET_PMIC_STATUS ), parts ) );
}

MESSAGE_PTR MESSAGE_PROCESSOR::create_set_status( const std::string& _board_tag, uint8_t _status ) throw( exception )
{
	vector<string> parts;
	parts.push_back( _board_tag );
	parts.push_back( num_to_str( _status ) );
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::SET_STATUS ), parts ) );
}

MESSAGE_PTR MESSAGE_PROCESSOR::create_get_raw_adc_values( const std::string& _board_tag ) throw( exception )
{
	vector<string> parts;
	parts.push_back( _board_tag );
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::READ_STATUS_RAW_ANALOG ), parts ) );
}

MESSAGE_PTR MESSAGE_PROCESSOR::create_get_status( const std::string& _board_tag ) throw( exception )
{
	vector<string> parts;
	parts.push_back( _board_tag );
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::READ_STATUS ), parts ) );
}

MESSAGE_PTR MESSAGE_PROCESSOR::create_hello_message( void )
{
	vector<string> parts;
	parts.push_back( "VERSION" );
	parts.push_back( num_to_str( MESSAGE_PROCESSOR::MAX_SUPPORTED_PROTOCOL ) );
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::HELLO ), parts ) );
}

MESSAGE_PTR MESSAGE_PROCESSOR::create_ping_message( void )
{
	vector<string> parts;
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::PING ), parts ) );
}

MESSAGE_PTR MESSAGE_PROCESSOR::create_pong_message( void )
{
	vector<string> parts;
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::PONG ), parts ) );
}
MESSAGE_PTR MESSAGE_PROCESSOR::create_error( int _code, const std::string& _message ) throw( exception )
{
	vector<string> parts;
	parts.push_back( num_to_str( _code ) );
	parts.push_back( _message );
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::ERROR ), parts ) );
}

MESSAGE_PTR MESSAGE_PROCESSOR::create_get_l1_cal_vals( const std::string& _board_tag ) throw( exception )
{
	vector<string> parts;
	parts.push_back( _board_tag );
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::GET_L1_CAL_VALS ), parts ) );
}

MESSAGE_PTR MESSAGE_PROCESSOR::create_get_l2_cal_vals( const std::string& _board_tag ) throw( exception )
{
	vector<string> parts;
	parts.push_back( _board_tag );
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::GET_L2_CAL_VALS ), parts ) );
}

MESSAGE_PTR MESSAGE_PROCESSOR::create_set_l1_cal_vals( const std::string& _board_tag, const CAL_VALUE_ARRAY& _vals ) throw( exception )
{
	vector<string> parts;
	parts.push_back( _board_tag );
	convert_vector_to_string( _vals, parts );
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::SET_L1_CAL_VALS ), parts ) );
}

MESSAGE_PTR MESSAGE_PROCESSOR::create_set_l2_cal_vals( const std::string& _board_tag, const CAL_VALUE_ARRAY& _vals ) throw( exception )
{
	vector<string> parts;
	parts.push_back( _board_tag );
	convert_vector_to_string( _vals, parts );
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::SET_L2_CAL_VALS ), parts ) );
}

MESSAGE_PTR MESSAGE_PROCESSOR::create_get_boot_count( const std::string& _board_tag ) throw( exception )
{
	vector<string> parts;
	parts.push_back( _board_tag );
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::GET_BOOT_COUNT ), parts ) );
}

MESSAGE_PTR MESSAGE_PROCESSOR::create_force_an( const std::string& _board_tag, uint8_t _input, uint16_t _value ) throw ( exception )
{
	vector<string> parts;
	parts.push_back( _board_tag );
	parts.push_back( num_to_str( _input ) );
	parts.push_back( num_to_str( _value ) );
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::FORCE_AI_VALUE ), parts ) );
}
MESSAGE_PTR MESSAGE_PROCESSOR::create_unforce_force_an( const std::string& _board_tag, uint8_t _input ) throw ( exception )
{
	vector<string> parts;
	parts.push_back( _board_tag );
	parts.push_back( num_to_str( _input ) );
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::UNFORCE_AI_VALUE ), parts ) );
}

MESSAGE_PTR MESSAGE_PROCESSOR::get_latest_outgoing_ping( void )
{
	for ( MESSAGE_VECTOR::reverse_iterator i = this->outgoing_message_queue->messages.rbegin(); i != this->outgoing_message_queue->messages.rend(); ++i )
	{
		if ( ( *i )->get_message_type()->type == ENUM_MESSAGE_TYPE::PING )
		{
			return *i;
		}
	}

	return MESSAGE_PTR();
}

MESSAGE_PTR MESSAGE_PROCESSOR::create_read_logic_status( void ) throw( exception )
{
	vector<string> parts;
	return MESSAGE_PTR( new MESSAGE( MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::READ_LOGIC_STATUS ), parts ) );
}

MESSAGE_PTR MESSAGE_PROCESSOR::get_latest_incomming_of_type( ENUM_MESSAGE_TYPE _type )
{
	//cout << "IQ: [" + this->incomming_message_queue->to_string() + "]" << endl;
	for ( MESSAGE_VECTOR::reverse_iterator i = this->incomming_message_queue->messages.rbegin(); i != this->incomming_message_queue->messages.rend(); ++i )
	{
		if ( ( *i )->get_message_type()->type == _type )
		{
			return *i;
		}
	}

	return MESSAGE_PTR();
}

MESSAGE_PTR MESSAGE_PROCESSOR::get_latest_incomming_pong( void )
{
	return this->get_latest_incomming_of_type( ENUM_MESSAGE_TYPE::PONG );
}

string MESSAGE_PROCESSOR::to_string( void ) const
{
	stringstream ret;
	ret << "*** begin MESSAGE_PROCESSOR instance:" << endl;
	ret << "IQ: " << this->incomming_message_queue->to_string() << endl;
	ret << "OQ: " << this->outgoing_message_queue->to_string() << endl;
	ret << "*** end MESSAGE_PROCESSOR instance:" << endl;
	return ret.str();
}

/****************************************
 *
 * MESSAGE_QUEUE stuff below
 *
 ****************************************/

MESSAGE_QUEUE::MESSAGE_QUEUE( unsigned int _size )
{
	this->size = _size;
}

MESSAGE_QUEUE::~MESSAGE_QUEUE()
{
	this->messages.clear();
	return;
}

size_t MESSAGE_QUEUE::get_message_count( void ) const
{
	return this->messages.size();
}

bool MESSAGE_QUEUE::has_messages( void ) const
{
	if ( this->messages.size() > 0 )
	{
		return true;
	}
	else
	{
		return false;
	}
}

MESSAGE_PTR MESSAGE_QUEUE::pop_first( void ) throw( exception )
{
	if ( !this->has_messages() )
	{
		throw ( EXCEPTIONS::MESSAGE_UNDERFLOW( "No messages available to pop." ) );
	}

	MESSAGE_PTR ret = this->messages.front();
	this->messages.erase( this->messages.begin() );
	return ret;
}

void MESSAGE_QUEUE::add_message( MESSAGE_PTR& _message, ENUM_APPEND_MODE _mode ) throw( exception )
{
	if ( this->messages.size() < this->size )
	{
		this->messages.push_back( _message );
	}
	else
	{
		switch ( _mode )
		{
			case ENUM_APPEND_MODE::LOSE_OVERFLOW:
				this->messages.erase( this->messages.begin() );
				this->messages.push_back( _message );
				break;

			case ENUM_APPEND_MODE::ERROR_OVERFLOW:
				throw EXCEPTIONS::MESSAGE_OVERFLOW( "Tried to add message to full queue." );
				break;
		}
	}

	return;
}

MESSAGE_PTR MESSAGE_QUEUE::get_message( unsigned int _idx ) throw( exception )
{
	if ( _idx >= this->messages.size() )
	{
		throw ( EXCEPTIONS::MESSAGE_ERROR( "Index out of bounds; too high." ) );
	}

	if ( this->messages.size() == 0 )
	{
		throw ( EXCEPTIONS::MESSAGE_ERROR( "Index out of bounds; no messages in queue." ) );
	}

	return this->messages[_idx];
}

string MESSAGE_QUEUE::to_string( void ) const
{
	stringstream ret;
	ret << "[";
	vector<string> v;

	for ( MESSAGE_VECTOR::const_iterator i = this->messages.cbegin(); i != this->messages.cend(); ++i )
	{
		v.push_back( "\n\t" + ( *i )->to_string() );
	}

	ret << join_vector( v, ',' );
	ret << endl << "]" << endl;
	return ret.str();
}
