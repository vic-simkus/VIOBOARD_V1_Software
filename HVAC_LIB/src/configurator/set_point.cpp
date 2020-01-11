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

SET_POINT::SET_POINT()
{
	this->description.clear();
	this->value = 0xFFFF;
	this->index = 0xFF;
}

SET_POINT::SET_POINT( const std::string& _description, double _value )
{
	this->description = _description;
	this->value = _value;
	this->index = 0xFE;
	return;
}

SET_POINT::SET_POINT( const std::string& _description, double _value, size_t _index )
{
	this->description = _description;
	this->value = _value;
	this->index = _index;
	return;
}

SET_POINT::SET_POINT( const SET_POINT& _src )
{
	this->description = _src.description;
	this->value = _src.value;
	this->index = _src.index;
	return;
}

SET_POINT::SET_POINT( const CONFIG_ENTRY& _config_entry, size_t _index )
{
	this->description = _config_entry.get_part_as_string( 0 );
	this->value = _config_entry.get_part_as_double( 1 );
	this->index = _index;
	return;
}

std::string SET_POINT::to_string( void ) const
{
	return "(" + CONFIG_ENTRY::type_to_string( ENUM_CONFIG_TYPES::SP ) + ',' + this->description + ',' + num_to_str( this->value ) + ")";
}

SET_POINT SET_POINT::from_string( const std::string& _source )
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

	if ( parts.size() != 3 )
	{
		return SET_POINT();
	}

	return SET_POINT( parts[1], stod( parts[2] ) );
}

std::string SET_POINT::to_string_static( const std::pair<std::string, BBB_HVAC::SET_POINT>& _pair )
{
	return _pair.second.to_string();
}

double SET_POINT::get_value( void ) const
{
	return this->value;
}

void SET_POINT::set_value( double _val )
{
	this->value = _val;
}

size_t SET_POINT::get_index( void ) const
{
	return this->index;
}

std::string SET_POINT::get_description( void ) const
{
	return description;
}