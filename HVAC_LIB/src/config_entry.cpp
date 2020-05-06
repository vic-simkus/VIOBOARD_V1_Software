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


#include "lib/configurator.hpp"
#include "lib/string_lib.hpp"
#include "lib/exceptions.hpp"

#include <stdexcept>
#include <sstream>

using namespace BBB_HVAC;
using namespace std;

/*
 * Lots of copy-and-paste coding here...
 */

CONFIG_ENTRY::CONFIG_ENTRY( ENUM_CONFIG_TYPES _type, const CONFIG_PARTS_TYPE& _parts )
{
	this->type = _type;
	this->parts = _parts;
	this->is_dirty = false;
	return;
}
CONFIG_ENTRY::~CONFIG_ENTRY()
{
	this->parts.clear();
	this->type = ENUM_CONFIG_TYPES::INVALID;
	return;
}

unsigned char CONFIG_ENTRY::get_part_as_uchar( size_t _idx ) const throw( exception )
{
	return ( unsigned char )this->get_part_as_int( _idx );
}
int CONFIG_ENTRY::get_part_as_int( size_t _idx ) const throw( exception )
{
	if ( _idx >= this->parts.size() )
	{
		throw out_of_range( "get_part: Specified index is >= size of parts vector" );
	}

	return stoi( this->parts[_idx] );
}
double CONFIG_ENTRY::get_part_as_double( size_t _idx ) const throw( exception )
{
	if ( _idx >= this->parts.size() )
	{
		throw out_of_range( "get_part: Specified index is >= size of parts vector" );
	}

	return stod( this->parts[_idx] );
}
bool CONFIG_ENTRY::get_part_as_bool( size_t _idx ) const throw( exception )
{
	if ( _idx >= this->parts.size() )
	{
		throw out_of_range( "get_part: Specified index is >= size of parts vector" );
	}

	if ( this->parts[_idx] == "T" || this->parts[_idx] == "t" || this->parts[_idx] == "TRUE" || this->parts[_idx] == "true" )
	{
		return true;
	}
	else
	{
		return false;
	}
}
string CONFIG_ENTRY::get_part_as_string( size_t _idx ) const throw( exception )
{
	if ( _idx >= this->parts.size() )
	{
		throw out_of_range( "get_part: Specified index is >= size of parts vector" );
	}

	return this->parts[_idx];
}

void CONFIG_ENTRY::set_part( size_t _idx, int _val ) throw( exception )
{
	if ( _idx >= this->parts.size() )
	{
		throw out_of_range( "set_part: Specified index is >= size of parts vector" );
	}

	this->is_dirty = true;
	this->parts[_idx] = num_to_str( _val );
	return;
}
void CONFIG_ENTRY::set_part( size_t _idx, double _val ) throw( exception )
{
	if ( _idx >= this->parts.size() )
	{
		throw out_of_range( "set_part: Specified index is >= size of parts vector" );
	}

	this->is_dirty = true;
	this->parts[_idx] = num_to_str( _val );
	return;
}
void CONFIG_ENTRY::set_part( size_t _idx, const string& _val ) throw( exception )
{
	if ( _idx >= this->parts.size() )
	{
		throw out_of_range( "set_part: Specified index is >= size of parts vector: " + num_to_str( this->parts.size() ) );
	}

	this->is_dirty = true;
	this->parts[_idx] = _val;
	return;
}

ENUM_CONFIG_TYPES CONFIG_ENTRY::get_type( void ) const
{
	return this->type;
}
bool CONFIG_ENTRY::get_is_dirty( void ) const
{
	return this->is_dirty;
}

ENUM_CONFIG_TYPES CONFIG_ENTRY::string_to_type( const string& _type ) throw( exception )
{
	if ( _type.length() == 0 )
	{
		THROW_EXCEPTION( runtime_error, "Called with type string empty." );
	}

	if ( _type.length() > 7 )
	{
		THROW_EXCEPTION( runtime_error, "Called with type string of unexpected length." );
	}

	if ( _type == "DO" )
	{
		return ENUM_CONFIG_TYPES::DO;
	}
	else if ( _type == "AI" )
	{
		return ENUM_CONFIG_TYPES::AI;
	}
	else if ( _type == "SP" )
	{
		return ENUM_CONFIG_TYPES::SP;
	}
	else if ( _type == "INVALID" )
	{
		return ENUM_CONFIG_TYPES::INVALID;
	}
	else if ( _type == "BOARD" )
	{
		return ENUM_CONFIG_TYPES::BOARD;
	}
	else if ( _type == "MAP" )
	{
		return ENUM_CONFIG_TYPES::MAP;
	}
	else
	{
		THROW_EXCEPTION( runtime_error, "Called with invalid type string: " + _type );
	}
}

string CONFIG_ENTRY::type_to_string( ENUM_CONFIG_TYPES _type ) throw( exception )
{
	switch ( _type )
	{
		case ENUM_CONFIG_TYPES::AI:
			return ( "AI" );
			break;

		case ENUM_CONFIG_TYPES::DO:
			return ( "DO" );
			break;

		case ENUM_CONFIG_TYPES::SP:
			return ( "SP" );
			break;

		case ENUM_CONFIG_TYPES::BOARD:
			return ( "BOARD" );
			break;

		case ENUM_CONFIG_TYPES::MAP:
			return ( "MAP" );
			break;

		case ENUM_CONFIG_TYPES::INVALID:
			return ( "INVALID" );
			break;

		default:
			throw runtime_error( "CONFIG_ENTRY::type_to_string encountered an unexpected code path." );
			break;
	}
}
string CONFIG_ENTRY::write_self_to_file( void ) const throw( exception )
{
	if ( this->type == ENUM_CONFIG_TYPES::INVALID )
	{
		throw runtime_error( "CONFIG_ENTRY::write_self_to_file was called while its type is INVALID" );
	}

	vector<string> ret;
	ret.push_back( type_to_string( this->type ) );
	ret.insert( ret.end(), this->parts.begin(), this->parts.end() );
	return join_vector( ret, '\t' );
}
size_t CONFIG_ENTRY::get_part_count( void ) const
{
	return this->parts.size();
}

std::string CONFIG_ENTRY::to_string( void ) const
{
	stringstream ss;
	vector<string> ret;
	ret.push_back( type_to_string( this->type ) );
	ret.insert( ret.end(), this->parts.begin(), this->parts.end() );
	ss << "(";
	ss << join_vector( ret, ',' );
	ss << ")";
	return ss.str();
}

bool CONFIG_ENTRY::operator==( const CONFIG_ENTRY& _other )
{
	if ( this->type != _other.type )
	{
		return false;
	}

	if ( this->parts.size() != _other.parts.size() )
	{
		return false;
	}

	for ( CONFIG_PARTS_TYPE::size_type i = 0; i < this->parts.size(); i++ )
	{
		if ( this->parts[i] != _other.parts[i] )
		{
			return false;
		}
	}

	return true;
}