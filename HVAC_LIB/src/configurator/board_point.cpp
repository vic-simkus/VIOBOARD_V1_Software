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

using namespace BBB_HVAC;


BOARD_POINT::BOARD_POINT()
{
	this->board_tag.clear();
	this->point_id = 0xFF;
	this->description.clear();
	this->type = ENUM_CONFIG_TYPES::INVALID;
	this->index = 0;
	this->ai_type = AI_TYPE::INVALID;
	this->min = 0;
	this->max = 0;
	return;
}

BOARD_POINT::BOARD_POINT( const BOARD_POINT& _src )
{
	this->board_tag = _src.board_tag;
	this->point_id = _src.point_id;
	this->description = _src.description;
	this->type = _src.type;
	this->index = _src.index;
	this->ai_type = _src.ai_type;
	this->min = _src.min;
	this->max = _src.max;
	this->is_celcius = _src.is_celcius;
	return;
}

BOARD_POINT::BOARD_POINT( const CONFIG_ENTRY& _config_entry, ENUM_CONFIG_TYPES _type, size_t _index )
{
	this->board_tag = _config_entry.get_part_as_string( 0 );
	this->point_id = _config_entry.get_part_as_uchar( 1 );
	this->description = _config_entry.get_part_as_string( 2 );
	this->type = _type;
	this->index = _index;
	this->is_celcius = false;

	if ( this->type == ENUM_CONFIG_TYPES::AI )
	{
		if ( _config_entry.get_part_as_string( 3 ) == "420" )
		{
			this->ai_type = AI_TYPE::CL_420;
			this->min = _config_entry.get_part_as_int( 4 );
			this->max = _config_entry.get_part_as_int( 5 );
		}
		else if ( _config_entry.get_part_as_string( 3 ) == "ICTD" )
		{
			this->ai_type = AI_TYPE::ICTD;
			this->is_celcius = ( _config_entry.get_part_as_string( 4 ) == "C" ) ? true : false;
		}

		//
		// Yeah fudge error handling a bit.
		//
	}

	return;
}

BOARD_POINT::~BOARD_POINT()
{
	this->board_tag.clear();
	this->point_id = 0xFF;
	this->description.clear();
	this->type = ENUM_CONFIG_TYPES::INVALID;
	this->index = 0;
	this->ai_type = AI_TYPE::INVALID;
	this->min = 0;
	this->max = 0;
	this->is_celcius = true;
	return;
}

const std::string& BOARD_POINT::get_board_tag( void ) const
{
	return this->board_tag;
}

const std::string& BOARD_POINT::get_description( void ) const
{
	return this->description;
}

unsigned char BOARD_POINT::get_point_id( void ) const
{
	return this->point_id;
}

ENUM_CONFIG_TYPES BOARD_POINT::get_type( void ) const
{
	return this->type;
}

size_t BOARD_POINT::get_index( void ) const
{
	return this->index;
}

std::string BOARD_POINT::to_string( void ) const
{
	return "(" + CONFIG_ENTRY::type_to_string( this->type ) + ',' + this->board_tag + ',' + num_to_str( this->point_id ) + ',' + this->description + ")";
}

BOARD_POINT BOARD_POINT::from_string( const std::string& _source )
{
	/*
	No fucking error checking at all...

	XXX - need to make this not shit.  Like add error checking and handling and other such luxuries.
	*/
	// Get rid of the perens.
	std::string str = trimmed( _source );
	str = str.substr( 1, _source.length() - 2 );
	std::vector<std::string> parts;
	split_string_to_vector( str, ',', parts );
	BOARD_POINT ret;

	if ( parts.size() != 4 )
	{
		return ret;
	}

	ret.type = CONFIG_ENTRY::string_to_type( parts[0] );
	ret.board_tag = parts[1];
	ret.point_id = ( unsigned char )stoi( parts[2] );
	ret.description = parts[3];
	return ret;
}

std::string BOARD_POINT::to_string_static( const BOARD_POINT& _point )
{
	return _point.to_string();
}

long BOARD_POINT::get_min_value( void ) const
{
	return this->min;
}

long BOARD_POINT::get_max_value( void ) const
{
	return this->max;
}

bool BOARD_POINT::get_is_celcius( void ) const
{
	return this->is_celcius;
}

const AI_TYPE& BOARD_POINT::get_ai_type( void ) const
{
	return this->ai_type;
}