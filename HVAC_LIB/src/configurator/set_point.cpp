#include "lib/configurator.hpp"

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