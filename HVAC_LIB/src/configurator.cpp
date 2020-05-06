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
#include "lib/exceptions.hpp"
#include "lib/config.hpp"
#include "lib/string_lib.hpp"

#include <unistd.h>		// getcwd
#include <stdlib.h>		// realpath, access
#include <string.h>		// memset
#include <limits.h> 	// PATH_MAX

#include <sys/types.h>	// open and various flags
#include <sys/stat.h>	//	"		"		"
#include <fcntl.h>		//	"		"		"

#include <stdexcept>	// exceptions

#include <stdio.h>		// fopen, fgetc

#include <vector>		// vector

using namespace BBB_HVAC;
using namespace std;

CONFIGURATOR::CONFIGURATOR( const string& _file )
{
	INIT_LOGGER( "BBB_HVAC::CONFIGURATOR" );
	this->file_name = _file;
	this->buffer = ( char* ) malloc( GC_BUFFER_SIZE );
	memset( this->buffer, 0, GC_BUFFER_SIZE );

	return;
}
CONFIGURATOR::~CONFIGURATOR()
{
	memset( this->buffer, 0, GC_BUFFER_SIZE );
	free( this->buffer );

	this->buffer = nullptr;
	this->config_entries.clear();

	return;
}

void CONFIGURATOR::normalize_file_names( void ) throw( exception )
{
	char* _buff = ( char* ) malloc( PATH_MAX );

	memset( _buff, 0, PATH_MAX );
	getcwd( _buff, PATH_MAX );
	this->cwd = string( _buff );

	memset( _buff, 0, PATH_MAX );
	realpath( this->file_name.data(), _buff );
	this->file_name = string( _buff );

	this->overlay_file_name = this->file_name + ".overlay";

	memset( _buff, 0, PATH_MAX );
	free( _buff );

	return;
}

void CONFIGURATOR::check_file_permissions( void ) throw( exception )
{
	if ( access( this->file_name.data(), F_OK ) != 0 )
	{
		throw runtime_error( "Configuration file [" + this->file_name + "] does not exist." );
	}

	if ( access( this->file_name.data(), R_OK ) != 0 )
	{
		throw runtime_error( "Configuration file [" + this->file_name + "] is not readable to us." );
	}

	if ( access( this->overlay_file_name.data(), F_OK ) != 0 )
	{
		LOG_DEBUG( this->overlay_file_name + " does not exist." );

		int fd = open( this->overlay_file_name.data(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR );

		if ( fd == -1 )
		{
			throw runtime_error( create_perror_string( "Failed to create overlay file" ) );
		}

		close( fd );
	}

	return;
}

void CONFIGURATOR::process_file( const char* _file_name ) throw( exception )
{
	LOG_DEBUG( "Processing file: " + string( _file_name ) );

	memset( buffer, 0, GC_BUFFER_SIZE );

	int c = 0;
	size_t buff_idx = 0;
	size_t line_idx = 1;
	char cin = 0;

	FILE* file = fopen( _file_name, "rt" );

	while ( ( c = fgetc( file ) ) != EOF )
	{
		cin = ( char ) c;

		if ( cin == '\n' )
		{
			/*
			 * End of the line
			 */
			this->buffer[buff_idx] = '\0';
			/*
			 * Process line here.
			 */
			//LOG_DEBUG("End of line [" + num_to_str(line_idx) + "] @ " + num_to_str(buff_idx));
			this->process_line( line_idx );
			memset( this->buffer, 0, buff_idx );
			buff_idx = 0;
			line_idx += 1;
		}
		else
		{
			this->buffer[buff_idx] = ( char ) cin;
			buff_idx += 1;
		}

		if ( buff_idx == GC_BUFFER_SIZE )
		{
			fclose( file );
			throw runtime_error( "Failed to find line terminator before exceeding buffer size." );
		}
	}

	/*
		LOG_DEBUG( "Point mappings:" );

		for ( auto i = this->point_map.begin(); i != this->point_map.end(); ++i )
		{
			LOG_DEBUG( i->first + " --> " + i->second.to_string() );
		}
	*/

	LOG_DEBUG( "Lines processed: " + num_to_str( line_idx ) );
	fclose( file );
	return;
}
void CONFIGURATOR::process_mapping( const CONFIG_ENTRY& _ce ) throw( exception )
{
	ENUM_CONFIG_TYPES type = CONFIG_ENTRY::string_to_type( _ce.get_part_as_string( 0 ) );
	std::string target_board = _ce.get_part_as_string( 1 );
	unsigned int target_point = ( unsigned int )_ce.get_part_as_int( 2 );
	BOARD_POINT* target_entry = nullptr;
	BOARD_POINT_VECTOR* point_list = nullptr;

	if ( type == ENUM_CONFIG_TYPES::DO )
	{
		point_list = &this->do_points;
	}
	else if ( type == ENUM_CONFIG_TYPES::AI )
	{
		point_list = &this->ai_points;
	}
	else
	{
		LOG_ERROR( "Invalid target entry type: " + _ce.get_part_as_string( 0 ) );
		return;
	}

	for ( auto i = point_list->begin(); i != point_list->end(); ++i )
	{
		if ( i->get_board_tag() == target_board && target_point == i->get_point_id() )
		{
			target_entry = &( *i );
		}
	}

	if ( target_entry == nullptr )
	{
		LOG_ERROR( "Failed to find target entry for: " + _ce.to_string() );
		return;
	}

	std::string key = _ce.get_part_as_string( 3 );
	this->point_map[key] = ( *target_entry );
}
void CONFIGURATOR::process_line( size_t _line_idx ) throw( exception )
{
	char* line = trim_string( this->buffer );
	size_t line_length = strlen( line );
	size_t prev_tab = 0;

	//LOG_DEBUG("Processing line: " + num_to_str(_line_idx) + ".  Length: " + num_to_str(line_length));

	if ( line_length == 0 || line[0] == '#' )
	{
		return;
	}

	char c;
	vector<string> line_parts;

	for ( size_t i = 0; i < line_length; i++ )
	{
		c = line[i];

		if ( c == '\t' )
		{
			line[i] = '\0';

			if ( strlen( ( line + prev_tab ) ) > 0 )
			{
				line_parts.push_back( ( line + prev_tab ) );
			}
			else
			{
				LOG_WARNING( "A zero-length part was found on line " + num_to_str( _line_idx ) + " around field " + num_to_str( line_parts.size() ) + ".  Possible double-tab." );
			}

			prev_tab = i + 1;
		}
	}

	line_parts.push_back( line + prev_tab );
	ENUM_CONFIG_TYPES type;

	try
	{
		type = CONFIG_ENTRY::string_to_type( line_parts[0] );
	}
	catch ( const exception& _e )
	{
		LOG_ERROR( string( "Failed to process configuration line " + num_to_str( _line_idx ) + ": " + _e.what() ) );
		return;
	}

	if ( type == ENUM_CONFIG_TYPES::INVALID )
	{
		LOG_ERROR( string( "An INVALID record type specified in the configuration file at line " + num_to_str( _line_idx ) + "?" ) );
		return;
	}

	line_parts.erase( line_parts.begin() );
	size_t ces = this->config_entries.size();
	CONFIG_ENTRY ce = CONFIG_ENTRY( type, line_parts );

	for ( auto i = this->config_entries.begin(); i != this->config_entries.end(); ++i )
	{
		if ( ce == ( *i ) )
		{
			this->config_entries.erase( i );
		}
	}

	this->config_entries.push_back( ce );

	switch ( type )
	{
		case ENUM_CONFIG_TYPES::DO:
		{
			this->do_points.push_back( BOARD_POINT( ce, type, ces ) );
			break;
		}

		case ENUM_CONFIG_TYPES::AI:
		{
			if ( line_parts.size() < 4 )
			{
				LOG_ERROR( "Malformed AI entry at line " + num_to_str( _line_idx )  + "; missing parts." );
			}
			else
			{
				if ( line_parts[3] == "420" )
				{
					if ( line_parts.size() != 6 )
					{
						LOG_ERROR( "Malformed AI entry at line " + num_to_str( _line_idx ) + "; Wrong number of parts.  Expecting 6, found: " + num_to_str( line_parts.size() ) );
						break;
					}
				}
				else if ( line_parts[3] == "ICTD" )
				{
					//
					// No other parts for ICTD.  We're just doing a health and welfare check.
					if ( line_parts.size() != 5 )
					{
						LOG_ERROR( "Malformed AI entry at line " + num_to_str( _line_idx ) + "; Wrong number of parts.  Expecting 5, found: " + num_to_str( line_parts.size() ) );
						break;
					}

					//
				}
				else
				{
					LOG_ERROR( "Malformed AI entry at line " + num_to_str( _line_idx ) + "; Unrecognized type: " + line_parts[3] );
					break;
				}

				this->ai_points.push_back( BOARD_POINT( ce, type, ces ) );
			}

			break;
		}

		case ENUM_CONFIG_TYPES::SP:
		{
			string key = ce.get_part_as_string( 0 );
			this->sp_points[key] = SET_POINT( ce, ces );
			break;
		}

		case ENUM_CONFIG_TYPES::MAP:
		{
			this->map_configs.push_back( ces );
			this->process_mapping( ce );
			break;
		}

		case ENUM_CONFIG_TYPES::BOARD:
		{
			this->board_configs.push_back( ces );
			break;
		}

		case ENUM_CONFIG_TYPES::INVALID:
		{
			//this is caught above.  This is here for the compiler warning.
			break;
		}
	}

	return;
}
void CONFIGURATOR::read_file( void ) throw( exception )
{
	this->normalize_file_names();
	LOG_DEBUG( "Configuration file: " + this->file_name );
	LOG_DEBUG( "Overlay file: " + this->overlay_file_name );
	this->check_file_permissions();

	struct stat stat_config;
	struct stat stat_overlay;

	memset( &stat_config, 0, sizeof( struct stat ) );
	memset( &stat_overlay, 0, sizeof( struct stat ) );

	if ( stat( this->file_name.data(), &stat_config ) != 0 )
	{
		throw runtime_error( create_perror_string( "Failed to stat main config file." ) );
	}

	if ( stat( this->overlay_file_name.data(), &stat_overlay ) != 0 )
	{
		throw runtime_error( create_perror_string( "Failed to stat config overlay file." ) );
	}

	this->process_file( this->file_name.data() );

	LOG_DEBUG( "Config modification time: " + num_to_str( stat_config.st_mtime ) + "; overlay modification time: " + num_to_str( stat_overlay.st_mtime ) );

	if ( stat_config.st_mtime <= stat_overlay.st_mtime )
	{
		//
		// Main config file is older than the overlay.
		// If the main config file is neweer than the overlay we do not read in the overlay.
		// Doing so makes setpoint changes in the configuration file impossible to change (without deleting the overlay file)
		// as the overlay file is read in after the configuration file.
		//

		LOG_INFO( "Reading in the overlay file because it is newer than the configuration file." );

		this->process_file( this->overlay_file_name.data() );
	}
	else
	{
		LOG_INFO( "Not reading overlay file because configuration file is newer." );
	}

	return;
}
void CONFIGURATOR::write_file( void ) const throw( exception )
{
	FILE* out_file = fopen( this->overlay_file_name.data(), "wt" );

	fprintf( out_file, "#\n# This file is mechanically generated.  Manual edits will likely be lost.\n#\n" );

	unsigned int line_idx = 0;

	for ( CONFIG_ENTRY_LIST_TYPE::const_iterator i = this->config_entries.cbegin(); i != this->config_entries.cend(); ++i )
	{
		LOG_DEBUG( "Processing config entry: " + num_to_str( line_idx ) );

		if ( i->get_type() == ENUM_CONFIG_TYPES::SP )
		{
			fprintf( out_file, "%s\n", i->write_self_to_file().data() );
		}

		line_idx += 1;
	}

	fprintf( out_file, "# EOF\n" );
	fclose( out_file );

	return;
}

const CONFIG_TYPE_INDEX_TYPE& CONFIGURATOR::get_board_index( void ) const
{
	return this->board_configs;
}

CONFIG_ENTRY& CONFIGURATOR::get_config_entry( size_t _idx ) throw( exception )
{
	if ( _idx > this->config_entries.size() )
	{
		throw runtime_error( string( "Supplied index " + num_to_str( _idx ) + " is out of range." ) );
	}

	return this->config_entries[_idx];
}

BOARD_POINT_VECTOR CONFIGURATOR::get_do_points( void ) const
{
	return this->do_points;
}

/**
Returns the AI (analog input) configurations entries in the instance.
*/
BOARD_POINT_VECTOR CONFIGURATOR::get_ai_points( void ) const
{
	return this->ai_points;
}

/**
Returns the SP (set ppoint) configuration entires in the instance.  The key is the label of the set point.
*/
const SET_POINT_MAP& CONFIGURATOR::get_sp_points( void ) const
{
	return this->sp_points;
}

/**
Returns the value of the specified set point.  An exception is thrown if a set point with the specified name does not exist.
*/
double CONFIGURATOR::get_sp_value( const string& _name ) const throw( exception )
{
	return this->sp_points.at( _name ).get_value();
}

/**
Sets the value of the specified set point.  An exception is thrown if a set point with the specified name does not exist.
*/
void CONFIGURATOR::set_sp_value( const string& _name, double _value ) throw( exception )
{
	SET_POINT& sp = this->sp_points.at( _name );
	sp.set_value( _value );
	CONFIG_ENTRY& ce = this->config_entries.at( sp.get_index() );
	ce.set_part( 1, _value );
	return;
}

/**
Returns the point map.
\see CONFIGURATOR::point_map
*/
const std::map<std::string, BOARD_POINT>& CONFIGURATOR::get_point_map( void ) const
{
	return this->point_map;
}