/*
Copyright (C) 2019  Vidas Simkus (vic.simkus@simkus.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "include/Context.hpp"
#include "lib/string_lib.hpp"
#include "lib/exceptions.hpp"


/*
For stat, opendir, readdir
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*
For realpath
*/
#include <stdlib.h>

/*
For string?
*/
#include <string>

/*
For exceptions.
*/
#include <exception>

/*
For opendir
*/
#include <dirent.h>

/*
For errno
*/
#include <errno.h>

/*
For unique_ptr
*/
#include <memory>

/*
For ostream
*/
#include <ostream>

using namespace HMI_DATA_LOGGER;

bool Context::check_data_dir( void )
{
	this->full_data_dir.clear();
	std::unique_ptr<char> _data_dir( realpath( this->configuration.log_dir.data(), nullptr ) );
	std::string data_dir;

	if ( _data_dir )
	{
		data_dir = _data_dir.get();
	}
	else
	{
		LOG_ERROR( "Failed to normalize data dir: " + this->configuration.log_dir + ".  System error to follow." );
		LOG_ERROR( BBB_HVAC::create_perror_string( "Failed to normalize data dir: " ) );
		return false;
	}

	LOG_DEBUG( "Checking data dir: " + data_dir );
	struct stat stat_buff;
	int rc = stat( data_dir.data(), &stat_buff );

	if ( rc != 0 )
	{
		LOG_ERROR( BBB_HVAC::create_perror_string( "Failed to stat [" + data_dir + "]: " ) );
		return false;
	}

	if ( !S_ISDIR( stat_buff.st_mode ) )
	{
		LOG_ERROR( "Apparently " + data_dir + " is not a directory?" );
		return false;
	}

	LOG_DEBUG( "Directory check passed.  Returning true" );
	this->full_data_dir = data_dir;
	return true;
}

size_t Context::get_next_data_file_index( void ) throw( std::runtime_error )
{
	size_t next_file_index = 0;
	std::string pre_index_fluff = this->get_data_file_name_before_index();
	std::string post_index_fluff = this->get_data_file_name_after_index();
	size_t post_index_fluff_start = 0;

	if ( this->full_data_dir.empty() )
	{
		throw std::runtime_error( "Must call this->check_data_dir first.  Oh and also the call must succeed." );
	}

	DIR* dir_stream = opendir( this->full_data_dir.data() );

	if ( dir_stream == nullptr )
	{
		LOG_ERROR( BBB_HVAC::create_perror_string( "Failed to open data directory " + this->full_data_dir + ": " ) );
		throw std::runtime_error( BBB_HVAC::create_perror_string( "Failed to open data directory " + this->full_data_dir + ": " ) );
	}

	while ( 1 )
	{
		/*
		On error to readdir errno is changed.  We set it here to a known state.
		*/
		errno = 0;
		struct dirent* directory_entry = readdir( dir_stream );

		if ( directory_entry == nullptr )
		{
			if ( errno == 0 )
			{
				/*
				End of directory entries.
				*/
				break;
			}
			else
			{
				/*
				We have a problem.
				*/
				closedir( dir_stream );
				throw std::runtime_error( BBB_HVAC::create_perror_string( "Failed to get next directory entry: " ) );
			}
		}

		std::string entry_string( directory_entry->d_name );

		if ( entry_string == "." || entry_string == ".." )
		{
			continue;
		}

		//LOG_DEBUG_STAT("Processing directory entry: " + entry_string);

		if ( entry_string.find( pre_index_fluff ) == std::string::npos )
		{
			/*
			The beginning of the file name does not look like the pre-index portion.
			*/
			continue;
		}

		if ( ( post_index_fluff_start = entry_string.rfind( post_index_fluff ) ) == std::string::npos )
		{
			/*
			While the file starts with the configured data file preamble, it does not end with the expected suffix.
			*/
			continue;
		}

		//LOG_DEBUG( "Found a recognized file: " + entry_string );
		std::string str_index = entry_string.substr( pre_index_fluff.length(), entry_string.length() - ( pre_index_fluff.length() + post_index_fluff.length() ) );

		if ( str_index.length() < 1 )
		{
			LOG_WARNING( "Weirdness (1) in file name in data directory: [" + entry_string +  "].  An investigation is appropriate." );
			continue;
		}

		unsigned long file_index = 0;

		try
		{
			file_index = std::stoul( str_index );
		}
		catch ( const std::invalid_argument& e )
		{
			LOG_WARNING( "Weirdness (2) in file name in data directory: [" + entry_string + "].  An investigation is appropriate." );
			continue;
		}

		//LOG_DEBUG( "Index string: [" + str_index + "] --> [" + num_to_str( file_index ) + "]" );

		if ( file_index > next_file_index )
		{
			next_file_index = file_index;
		}
	}

	closedir( dir_stream );
	return next_file_index + 1;
}

std::string Context::get_data_file_name_before_index( void )
{
	size_t idx = this->configuration.base_data_file_name.find( '#' );

	if ( idx == std::string::npos )
	{
		/*
		Hash is not present.
		*/
		return this->configuration.base_data_file_name;
	}
	else
	{
		return this->configuration.base_data_file_name.substr( 0, idx );
	}
}

std::string Context::get_data_file_name_after_index( void )
{
	size_t idx = this->configuration.base_data_file_name.find( '#' );

	if ( idx == std::string::npos )
	{
		/*
		Hash is not present.
		*/
		return "";
	}
	else
	{
		return this->configuration.base_data_file_name.substr( idx + 1, this->configuration.base_data_file_name.length() );
	}
}

void Context::set_prog_name( const std::string& _p ) throw( std::logic_error )
{
	if ( !this->prog_name.empty() )
	{
		throw std::logic_error( "Tried to set prog_name more than once." );
	}

	this->prog_name = _p;
	return;
}

void Context::set_prog_name_fixed( const std::string& _p ) throw( std::logic_error )
{
	if ( !this->prog_name_fixed.empty() )
	{
		throw std::logic_error( "Tried to set prog_name_fixed more than once." );
	}

	this->prog_name_fixed = _p;
	return;
}

std::string Context::get_next_data_file_name( void )
{
	std::stringstream ss;
	ss << this->get_full_data_dir();
	ss << "/";
	ss << this->get_data_file_name_before_index();
	ss.fill( '0' );
	ss.width( 4 );
	ss << std::right << this->get_next_data_file_index();
	ss << this->get_data_file_name_after_index();
	return ss.str();
}

std::string Context::get_full_data_dir( void ) const
{
	return this->full_data_dir;
}


bool Context::get_new_file_flag( void )
{
	return this->new_file_flag;
}
void Context::reset_new_file_flag( void )
{
	this->new_file_flag = false;
}

std::ofstream& Context::get_output_stream( void )
{
	struct stat stat_buff;

	if ( this->configuration.rotate_size == 0 )
	{
		/*
		Do not rotate.
		*/
		return this->output_stream;
	}

	if ( stat( this->current_data_file_name.data(), &stat_buff ) != 0 )
	{
		throw ( runtime_error( BBB_HVAC::create_perror_string( "Failed to stat current log file: " ) ) );
	}

	size_t file_size_k = ( ( size_t )( stat_buff.st_size / 1024 ) );

	//LOG_DEBUG("Current log size in K: " + num_to_str(file_size_k));

	if ( file_size_k >= this->configuration.rotate_size )
	{
		this->close_output_stream();
		this->open_output_stream();
	}

	return this->output_stream;
}

void Context::open_output_stream( void ) throw( exception )
{
	if ( this->output_stream.is_open() )
	{
		LOG_ERROR( "Tried to open output stream more than once.  Check program logic." );
		return;
	}

	/*
	Why do I have to specify that a stream should seek to the end before each write if I already specify the mode 'ate' (seek to the end of stream immediately after open)
	C++ never fails to surprise.
	*/
	this->current_data_file_name = this->get_next_data_file_name();
	this->output_stream = std::ofstream( this->current_data_file_name, std::ios::binary | std::ios::out | std::ios::ate | std::ios::app );

	if ( !this->output_stream.good() )
	{
		throw ( runtime_error( "Failed to open output stream." ) );
	}

	this->new_file_flag = true;
	return;
}
void Context::close_output_stream( void )
{
	if ( this->output_stream.is_open() )
	{
		this->output_stream.close();
	}

	return;
}

Context HMI_DATA_LOGGER::create_context( int _argc, const char** _argv )
{
	Context ctx;

	if ( !create_configuration( ctx, _argc, _argv ) )
	{
		throw ( runtime_error( "Failed to configure application.  Check log output for details." ) );
	}

	return ctx;
}