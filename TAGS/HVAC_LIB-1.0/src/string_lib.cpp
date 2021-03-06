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


#include "lib/string_lib.hpp"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <sstream>

#include <iostream>
#include <iomanip>

/*
 * Hurray C++ standard libary
 */
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

char* trim_string_left( char* str )
{
	if ( str == 0 )
	{
		return str;
	}

	size_t str_len = strlen( str );

	if ( str_len == 0 )
	{
		return str;
	}

	size_t i = 0;

	for ( i = 0; i < strlen( str ); i++ )
	{
		if ( isspace( str[i] ) == 0 )
		{
			break;
		}
	}

	size_t i2 = 0;

	if ( i > 0 )
	{
		/*
		 * Move the whole string before the begining white space.
		 */
		for ( i2 = i; i2 < str_len; i2++ )
		{
			char c = str[i2];
			size_t ni = i2 - i;
			str[ni] = c;
		}

		str[i2 - i] = 0;
	}

	return str;
}

char* trim_string_right( char* str )
{
	if ( str == 0 )
	{
		return str;
	}

	size_t str_len = strlen( str );

	if ( str_len == 0 )
	{
		return str;
	}

	size_t i = 0;

	for ( i = str_len; i > 0; i-- )
	{
		if ( str[i] != 0 && isspace( str[i] ) == 0 )
		{
			break;
		}
	}

	str[i + 1] = 0;
	return str;
}

char* trim_string( char* str )
{
	return trim_string_left( trim_string_right( str ) );
}

std::string join_vector( const std::vector<std::string>& _vect, char _char )
{
	std::stringstream ret;

	for ( std::vector<std::string>::const_iterator it = _vect.begin(); it != _vect.end(); ++it )
	{
		ret << *it;

		if ( it + 1 != _vect.end() )
		{
			ret << _char;
		}
	}

	return ret.str();
}

std::string join_list( const std::list<std::string>& _list, char _char )
{
	std::stringstream ret;

	/*
	Fucking pain in the ass C++.  I can look at the next item in the iterator in the vector but not list.
	*/
	size_t i = 0;

	for ( auto it = _list.begin(); it != _list.end(); ++it )
	{
		ret << *it;

		if ( ( i + 1 ) < _list.size() )
		{
			ret << _char;
		}

		i += 1;
	}

	return ret.str();
}


const char* TEST_STR[] = { "   TESTING", "TESTING   ", "   TESTING   ", "  ", "", " ", "..A..", "T E S T I N G", 0 };

bool test_string_lib( void )
{
	std::cout << "Testing string lib." << std::endl;
	char* str = 0;
	int i = 0;

	while ( TEST_STR[i] != 0 )
	{
		str = ( char* ) malloc( strlen( TEST_STR[i] ) );
		strcpy( str, TEST_STR[i] );
		printf( "1 -- [%s]\n", trim_string( str ) );
		free( str );
		i += 1;
	}

	printf( "Done testing.\n" );
	return true;
}

std::string num_to_str( long _i )
{
	std::stringstream ss;
	ss << _i;
	return ss.str();
}

std::string num_to_str( unsigned long _i )
{
	std::stringstream ss;
	ss << _i;
	return ss.str();
}

std::string num_to_str( int _i )
{
	return num_to_str( ( long ) _i );
}

std::string num_to_str( unsigned int _i )
{
	return num_to_str( ( unsigned long ) _i );
}

std::string num_to_str( float _i )
{
	return num_to_str( ( double ) _i );
}

std::string num_to_str( double _i )
{
	std::stringstream ss;
	ss << _i;
	return ss.str();
}

std::string num_to_str( bool _i )
{
	if ( _i )
	{
		return "TRUE";
	}
	else
	{
		return "FALSE";
	}
}

std::string get_iso_date_time( void )
{
	std::stringstream ret;
	time_t t = time( nullptr );
	tm ts;
	localtime_r( &t, &ts );
	ret << ( ts.tm_year + 1900 ) << "-" << std::setw( 2 ) << std::setfill( '0' ) << ( ts.tm_mon + 1 ) << "-" << std::setw( 2 ) << std::setfill( '0' ) << ts.tm_mday << "T" << std::setw( 2 ) << std::setfill( '0' ) << ts.tm_hour << ":" << std::setw( 2 ) << std::setfill( '0' ) << ts.tm_min << ":" << std::setw( 2 ) << std::setfill( '0' ) << ts.tm_sec;
	//
	return ret.str();
}

// trim from start (in place)

void ltrim( std::string& s )
{
	// This compiles fine with clang8 on Linux, but not on FreeBSD...
	//s.erase( s.begin(), std::find_if( s.begin(), s.end(), std::not1( std::ptr_fun<int, int> ( std::isspace ) ) ) );
	s.erase( s.begin(), std::find_if( s.begin(), s.end(), std::not1( std::function<int(int)> ( isspace ) ) ) );
}

// trim from end (in place)

void rtrim( std::string& s )
{
	// This compiles fine with clang8 on Linux, but not on FreeBSD...
	//s.erase( std::find_if( s.rbegin(), s.rend(), std::not1( std::ptr_fun<int, int> ( std::isspace ) ) ).base(), s.end() );
	s.erase( std::find_if( s.rbegin(), s.rend(), std::not1( std::function<int(int)> ( isspace ) ) ).base(), s.end() );
}

// trim from both ends (in place)

void trim( std::string& s )
{
	ltrim( s );
	rtrim( s );
}

// trim from start (copying)

std::string ltrimmed( std::string s )
{
	ltrim( s );
	return s;
}

// trim from end (copying)

std::string rtrimmed( std::string s )
{
	rtrim( s );
	return s;
}

// trim from both ends (copying)

std::string trimmed( std::string s )
{
	trim( s );
	return s;
}

std::string to_upper_case( const std::string& _in )
{
	std::string ret = _in;
	std::transform( ret.begin(), ret.end(), ret.begin(), ::toupper );
	return ret;
}

std::string char_to_hex( const unsigned char _num )
{
	std::stringstream out;
	out << std::setw( 2 ) << std::setfill( '0' ) << std::hex << ( int ) _num;
	return to_upper_case( out.str() );
}

std::string int_to_hex( const uint16_t _num )
{
	std::stringstream out;
	out << std::setw( 4 ) << std::setfill( '0' ) << std::hex << ( int ) _num;
	return to_upper_case( out.str() );
}

std::string buffer_to_hex( const unsigned char* _buff, const size_t _length, bool _fancy )
{
	std::stringstream out;

	for ( size_t i = 0; i < _length; i++ )
	{
		out << "0x" << char_to_hex( _buff[i] );

		if ( i + 1 < _length )
		{
			out << ":";
		}

		if ( _fancy )
		{
			if ( i > 0 && i % 8 == 0 )
			{
				out << std::endl;
			}
		}
	}

	out << std::endl;

	for ( size_t i = 0; i < _length; i++ )
	{
		out << "0x" << char_to_hex( ( unsigned char ) i );

		if ( i + 1 < _length )
		{
			out << ":";
		}

		if ( _fancy )
		{
			if ( i > 0 && i % 8 == 0 )
			{
				out << std::endl;
			}
		}
	}

	return out.str();
}

ssize_t find_last_index_of( const unsigned char* _str, const char _chr, const ssize_t _str_len )
{
	ssize_t i = _str_len;

	for ( ; i != 0; i-- )
	{
		if ( _str[i] == _chr )
		{
			return ( ssize_t ) i;
		}
	}

	return ( -1 );
}

ssize_t find_index_of( const unsigned char* _str, const char _chr, const ssize_t _start_idx, const ssize_t _str_len )
{
	if ( _start_idx >= _str_len )
	{
		return -1;
	}

	ssize_t i = _start_idx;

	for ( ; i < _str_len; i++ )
	{
		if ( _str[i] == _chr )
		{
			return i;
		}
	}

	return i;
}

void convert_vector_to_string( const std::vector<int>& _source, std::vector<std::string>& _dest )
{
	for ( std::vector<int>::const_iterator i = _source.begin(); i != _source.end(); ++i )
	{
		_dest.push_back( num_to_str( *i ) );
	}

	return;
}

void convert_vector_to_string( const std::vector<uint16_t>& _source, std::vector<std::string>& _dest )
{
	for ( std::vector<uint16_t>::const_iterator i = _source.begin(); i != _source.end(); ++i )
	{
		_dest.push_back( num_to_str( *i ) );
	}

	return;
}

uint16_t checksum( const uint16_t* _buffer, size_t _length )
{
	uint32_t sum = 0;

	/*
	 * IP headers always contain an even number of bytes.
	 */
	while ( _length -- > 0 )
	{
		sum += * ( _buffer++ );
	}

	/*
	 * Use carries to compute 1's complement sum.
	 */
	sum = ( sum >> 16 ) + ( sum & 0xFFFF );
	sum += sum >> 16;
	/*
	 * Return the inverted 16-bit result.
	 */
	return ( ( uint16_t ) ~sum );
}

void split_string_to_vector( const std::string& _string, char _split, std::vector<std::string>& _vector )
{
	size_t previous_match = 0;
	size_t current_match = 0;
	_vector.clear();

	while ( 1 )
	{
		current_match = _string.find( _split, previous_match );

		if ( current_match == std::string::npos )
		{
			_vector.push_back( _string.substr( previous_match, std::string::npos ) );
			break;
		}

		_vector.push_back( _string.substr( previous_match, current_match - previous_match ) );
		previous_match = current_match + 1;
	}

	return;
}

std::string byte_to_bit_string( uint8_t _n )
{
	std::string ret;

	for ( size_t i = ( ( ( ( size_t )sizeof( _n ) ) * 8 ) - 1 ); ; i-- )

	{
		if ( ( ( _n >> i ) & 1 ) )
		{
			ret += '1';
		}
		else
		{
			ret += '0';
		}

		if ( i == 0 )
		{
			break;
		}
	}

	return ret;
}
