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

#include "lib/message_lib.hpp"
#include "lib/string_lib.hpp"
#include "lib/exceptions.hpp"
#include "lib/hvac_types.hpp"

#include <string.h>
#include <sstream>
#include <iostream>

using namespace BBB_HVAC;

const char MESSAGE::sep_char = '|';

const timespec* MESSAGE::get_message_sent_timestamp( void ) const
{
	return this->message_sent;
}
const timespec* MESSAGE::get_message_created_timestamp( void ) const
{
	return this->class_created;
}
const timespec* MESSAGE::get_message_received_timestamp( void ) const
{
	return this->message_received;
}

MESSAGE::MESSAGE( const MESSAGE_TYPE& _type, const vector<string>& _payload )
{
	init();
	this->message_type = _type;
	this->parts = _payload;
	this->build_message();
	get_timestamp( this->class_created );
}

void MESSAGE::get_timestamp( timespec* _tm ) throw( runtime_error )
{
	if( clock_gettime( CLOCK_MONOTONIC, _tm ) != 0 )
	{
		throw runtime_error( create_perror_string( "Failed to get timestamp from system clock" ) );
	}

	return;
}
void MESSAGE::build_message( void )
{
	vector<string> v;
	v.push_back( this->message_type->label );
	v.insert( v.end(), this->parts.begin(), this->parts.end() );
	string pld = join_vector( v, MESSAGE::sep_char );
	size_t pld_length = pld.length();
	std::stringstream ret;
	ret << pld_length; //Get the length of the payload without the 'size|' preamble.
	size_t pld_length_length = ret.str().length();
	ret.str( "" );
	ret.clear();
	ret.seekp( std::ios_base::beg );
	ret << ( pld_length + pld_length_length );
	pld_length_length = ret.str().length();
	ret.str( "" );
	ret.clear();
	ret.seekp( std::ios_base::beg );
	/*
	 * Plus 2 because:
	 * +1 - the separator after length
	 * +1 - the new line at the end of message
	 */
	ret << ( pld_length_length + pld_length + 2 );
	ret << MESSAGE::sep_char << pld << std::endl;
	this->payload = ret.str();
	this->length = payload.length();
	return;
}

MESSAGE_TYPE MESSAGE::get_message_type( void ) const
{
	return this->message_type;
}
size_t MESSAGE::get_length( void ) const
{
	return this->length;
}
const string& MESSAGE::get_payload( void ) const
{
	return this->payload;
}
const vector<string>& MESSAGE::get_parts( void ) const
{
	return this->parts;
}

uint16_t MESSAGE::get_part_as_ui( size_t _part ) throw( exception )
{
	this->check_part_index( _part );

	try
	{
		return ( ( uint16_t ) stoi( this->parts[_part] ) );
	}
	catch( const exception& e )
	{
		throw runtime_error( string( "Failed to parse part " ) + this->parts[_part] + " to an unsigned integer: " + e.what() );
	}
}
int16_t MESSAGE::get_part_as_si( size_t _part ) throw( exception )
{
	this->check_part_index( _part );

	try
	{
		return ( ( int16_t ) stoi( this->parts[_part] ) );
	}
	catch( const exception& e )
	{
		throw runtime_error( string( "Failed to parse part " ) + this->parts[_part] + " to a signed integer: " + e.what() );
	}
}
string MESSAGE::get_part_as_s( size_t _part ) throw( exception )
{
	this->check_part_index( _part );
	return this->parts[_part];
}

size_t MESSAGE::get_part_count( void ) const
{
	return this->parts.size();
}
void MESSAGE::check_part_index( size_t _idx ) throw( exception )
{
	if( this->parts.size() == 0 || _idx >= this->parts.size() )
	{
		throw runtime_error( string( "Supplied part index is out of range.  Part count: " ) + num_to_str( ( unsigned int ) this->parts.size() ) + string( ", index: " ) + num_to_str( _idx ) );
	}
}

void MESSAGE::tag_received( void ) throw( runtime_error )
{
	if( this->message_received->tv_sec != 0 )
	{
		throw runtime_error( "Attempt was made to tag a message instance as received more than once." );
	}

	get_timestamp( this->message_received );
	return;
}

void MESSAGE::tag_sent( void ) throw( runtime_error )
{
	if( this->message_sent->tv_sec != 0 )
	{
		throw runtime_error( "Attempt was made to tag a message instance as sent more than once." );
	}

	get_timestamp( this->message_sent );
	return;
}

MESSAGE::~MESSAGE()
{
	this->length = 0;
	this->message_type = MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::INVALID );
	this->payload.clear();
	this->parts.clear();
	memset( this->class_created, 0, sizeof( struct timespec ) );
	memset( this->message_received, 0, sizeof( struct timespec ) );
	memset( this->message_sent, 0, sizeof( struct timespec ) );
	delete this->class_created;
	delete this->message_received;
	delete this->message_sent;
}
void MESSAGE::init( void )
{
	this->length = 0;
	this->message_type = MESSAGE_TYPE_MAPPER::get_message_type_by_enum( ENUM_MESSAGE_TYPE::INVALID );
	this->payload.clear();
	this->parts.clear();
	this->class_created = new timespec();
	this->message_received = new timespec();
	this->message_sent = new timespec();
	memset( this->class_created, 0, sizeof( struct timespec ) );
	memset( this->message_received, 0, sizeof( struct timespec ) );
	memset( this->message_sent, 0, sizeof( struct timespec ) );
	return;
}

/*
void MESSAGE::trim_message(void)
{
	if (this->message_type->type == ENUM_MESSAGE_TYPE::GET_LABELS)
	{
		this->parts.erase(this->parts.begin(), this->parts.begin() + 2);
	}

	return;
}
 */
string MESSAGE::to_string( void ) const
{
	string ret;
	string created_ts;
	string received_ts;
	string sent_ts;
	stringstream ss;
	ss << this->class_created->tv_sec << "." << this->class_created->tv_nsec;
	created_ts = ss.str();
	ss.str( "" );
	ss.clear();
	ss.seekp( ios_base::beg );
	ss << this->message_received->tv_sec << "." << this->message_received->tv_nsec;
	received_ts = ss.str();
	ss.str( "" );
	ss.clear();
	ss.seekp( ios_base::beg );
	ss << this->message_sent->tv_sec << "." << this->message_sent->tv_nsec;
	sent_ts = ss.str();
	ss.str( "" );
	ss.clear();
	ss.seekp( ios_base::beg );
	string p = join_vector( this->parts, ':' );
	ret = "(MSG:" + this->message_type->label + "; c:" + created_ts + "; r:" + received_ts + "; s:" + sent_ts + "; (" + p + "))";
	return ret;
}

void MESSAGE::message_to_map( const MESSAGE_PTR& _message, std::map<std::string, std::string>& _dest_map ) throw( exception )
{
	if( _message->get_part_count() < 2 )
	{
		throw logic_error( "Message part count too low." );
	}

	if( ( _message->get_part_count() % 2 ) != 0 )
	{
		throw logic_error( "Message part count is not even." );
	}

	for( size_t i = 0; i < _message->get_part_count(); i += 2 )
	{
		_dest_map.emplace( std::make_pair( _message->get_part_as_s( i ), _message->get_part_as_s( i + 1 ) ) );
	}

	return;
}