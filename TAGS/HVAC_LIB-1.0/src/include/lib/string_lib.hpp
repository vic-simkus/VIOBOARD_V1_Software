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


#ifndef STRING_LIB_H_
#define STRING_LIB_H_

#include <vector>
#include <list>
#include <string>

/*\file Stuff here is to get around the fucking travesty that is the standard C++ library.
 */

/**
 * Trims the white space from the start and end of a given buffer.
 * \param str Buffer containing the string to be trimmed.  The contents of the buffer will be modified.
 * \return The same pointer as passed in the str parameter.
 */
char* trim_string( char* str );

/**
 * Trims the white space from the start of the given buffer.
 * \param str Buffer containing the string to be trimmed.  The contents of the buffer will be modified.
 * \return The same pointer as passed in the str parameter.
 */
char* trim_string_left( char* str );

/**
 * Trims the white space from the end of a given buffer.
 * \param str Buffer containing the string to be trimmed.  The contents of the buffer will be modified.
 * \return The same pointer as passed in the str parameter.
 */
char* trim_string_right( char* str );

/**
 * Joins the supplied vector of strings into a single string separating the individual elements using the provided character.
 * \param _vect The string vector to be joined.
 * \param _char The separator character to be used between the elements of the vector.
 * \return The generated (joined) string.
 */
std::string join_vector( const std::vector<std::string>& _vect, char _char );

/**
 * Joins the supplied vector of strings into a single string separating the individual elements using the provided character.
 * \param _vect The string vector to be joined.
 * \param _char The separator character to be used between the elements of the vector.
 * \return The generated (joined) string.
 */
std::string join_list( const std::list<std::string>& _vect, char _char );

/**
	Converts the supplied byte (_n) into a binary string representation in little endian format.
	\param _n Byte to be converted
	\return A string containing binary representation.
*/
std::string byte_to_bit_string( uint8_t _n );
/**
 * Converts a number to a string.
 * \param _i Number to be converted
 * \return An std::string instance containing the string representation of the number.
 */
std::string num_to_str( int _i );

/**
 * \see num_to_str(int)
 */
std::string num_to_str( unsigned int _i );

/**
 * \see num_to_str(int)
 */
std::string num_to_str( long _i );

/**
 * \see num_to_str(int)
 */
std::string num_to_str( unsigned long _i );

/**
 * \see num_to_str(int)
 */
std::string num_to_str( float _i );

/**
 * \see num_to_str(int)
 */
std::string num_to_str( double _i );

/**
 * \see num_to_str(int)
 */
std::string num_to_str( bool _i );

/**
 * Returns the current date and time in ISO format.
 * \return Time and date in ISO format: YYYY-MM-DDTHH:MM:SS
 */
std::string get_iso_date_time( void );

/**
 * Trims the whitespace from the left side of the string.  Operation is done in place and supplied string is modified.
 * \param s String to be modified.
 */
void ltrim( std::string& s );

/**
 * Trims the whitespace from the right side of the string  Operation is done in place and supplied string is modified.
 * \param s String to be modified.
 */
void rtrim( std::string& s );

/**
 * Trims the string from the left and the right.  Operation is done in place and supplied string is modified.
 * \param s String to be modified.
 */
void trim( std::string& s );

/**
 * Trims the whitespace from the left side of the string.  Operation is done on a copy.  The original string is not modified.
 * \param s Original string.
 * \return Output string.
 */
std::string ltrimmed( std::string s );

/**
 * Trims the whitespace from the right side of the string.  Operation is done on a copy.  The original string is not modified.
 * \param s Original string.
 * \return Output string.
 */
std::string rtrimmed( std::string s );

/**
 * Trims the whitespace from both sides of the string.  Operation is done on a copy.  The original string is not modified.
 * \param s Original string.
 * \return Output string.
 */
std::string trimmed( const std::string s );

/**
 * Converts a string to uppercase.    Operation is done on a copy.  The original string is not modified.
 * \param _in Original string.
 * \return Output string.
 */
std::string to_upper_case( const std::string& _in );

/**
 * Converts a character to its hex representation.  The output is zero-padded two characters wide.
 * \param _num  Number to convert.
 * \return Output string.
 */
std::string char_to_hex( const unsigned char _num );

/**
 * Converts an integer to its hex representation.  The output is zer-padded four characters wide.
 * \param _num Number to convert.
 * \return Output string.
 */
std::string int_to_hex( const uint16_t _num );

/**
 * Converts a buffer (character array) to a string representing each of the characters hex value.  Each value is zero-padded two characters wide, with an 0x prefix, and a : separator.
 * \param _buff Buffer to convert.
 * \param _length Number of bytes in the buffer to convert.
 * \return Output string.
 */
std::string buffer_to_hex( const unsigned char* _buff, const size_t _length, bool _fancy = false );

/**
 * Tests the methods in the library.
 */
bool test_string_lib( void );

/**
 * \brief Find the last occurrence of a character within a string.
 * \param _str String buffer to search in.  Technically it doesn't have to be a string since no string-specific actions (such as looking for a zero termination) are done.
 * \param _chr Character to search for.
 * \param _str_len Length of the string to search through.  In other words the upper bound of the search as the parameter can be used to limit the scope of the search.
 * \return Index of the last occurrence of the character or -1 otherwise.
 */
ssize_t find_last_index_of( const unsigned char* _str, const char _chr, ssize_t _str_len );

/**
 * Find the first occurance of a character within a string.
 * \param _str The string buffer to search in.  Need not be a 'real' string.
 * \param _chr Character to search for.
 * \param _start_idx Index at which to start the search.
 * \param _str_len  Length of the string to search through or the upper bound of the search.
 * \return Index of the first occurrence past _start_index of the specified character.  If not found -1 is returned.
 */
ssize_t find_index_of( const unsigned char* _str, const char _chr, const ssize_t _start_idx, const ssize_t _str_len );

void convert_vector_to_string( const std::vector<int>& _source, std::vector<std::string>& _dest );
void convert_vector_to_string( const std::vector<uint16_t>& _source, std::vector<std::string>& _dest );

uint16_t checksum( const uint16_t* _buffer, size_t _length );

void split_string_to_vector( const std::string& _string, char _split, std::vector<std::string>& _vector );

#endif /* STRING_LIB_H_ */
