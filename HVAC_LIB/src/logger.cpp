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

#include "lib/logger.hpp"
#include "lib/string_lib.hpp"

using namespace BBB_HVAC::LOGGING;

#include <iostream>
#include <ostream>
#include <sstream>

#include <unistd.h>

const string LEVEL_NAMES[] = { "INVALID", "NONE", "TRACE", "DEBUG", "INFO", "WARNING", "ERROR" };

LOG_CONFIGURATOR* LOG_CONFIGURATOR::root_configurator = nullptr;

LOG_CONFIGURATOR::LOG_CONFIGURATOR( ENUM_LOG_LEVEL _level ) : TPROTECT_BASE( "LOG_CONFIGURATOR" )
{
	this->level = _level;

	if( LOG_CONFIGURATOR::root_configurator == nullptr )
	{
		LOG_CONFIGURATOR::root_configurator = this;
	}
}
LOG_CONFIGURATOR::~LOG_CONFIGURATOR()
{
	return;
}

ENUM_LOG_LEVEL LOG_CONFIGURATOR::get_level( void ) const
{
	return this->level;
}

LOG_CONFIGURATOR* LOG_CONFIGURATOR::get_root_configurator( void )
{
	return LOG_CONFIGURATOR::root_configurator;
}
void LOG_CONFIGURATOR::destroy_root_configurator( void )
{
	delete LOG_CONFIGURATOR::root_configurator;
	LOG_CONFIGURATOR::root_configurator = nullptr;
}

void LOG_CONFIGURATOR::log( const string& _log_name, const ENUM_LOG_LEVEL& _level, const string& _msg, const string& _file, int _line, const string& _function )
{
	this->obtain_lock_ex();
	this->output_buffer.str( "" );
	this->output_buffer.clear();
	this->output_buffer << get_iso_date_time() << " - [" << LEVEL_NAMES[static_cast<unsigned int> ( _level )] << "] " << _log_name << ":" << _file << "@" << _line << ":" << _function << " -- " << _msg << endl;
	string str = this->output_buffer.str();
	write( STDERR_FILENO, str.data(), str.length() );
	this->release_lock();
	return;
}

LOGGER::LOGGER( const string& _name )
{
	this->name = _name;
	this->level = ENUM_LOG_LEVEL::INVALID;
	return;
}
LOGGER::LOGGER()
{
	this->name = "NONAME";
	this->level = ENUM_LOG_LEVEL::INVALID;
	return;
}
void LOGGER::log_debug( const string& _msg, const string& _file, int _line, const string& _function )
{
	this->log( ENUM_LOG_LEVEL::DEBUG, _msg, _file, _line, _function );
}
void LOGGER::log_info( const string& _msg, const string& _file, int _line, const string& _function )
{
	this->log( ENUM_LOG_LEVEL::INFO, _msg, _file, _line, _function );
}
void LOGGER::log_trace( const string& _msg, const string& _file, int _line, const string& _function )
{
	this->log( ENUM_LOG_LEVEL::TRACE, _msg, _file, _line, _function );
}
void LOGGER::log_error( const string& _msg, const string& _file, int _line, const string& _function )
{
	this->log( ENUM_LOG_LEVEL::ERROR, _msg, _file, _line, _function );
}
void LOGGER::log_warning( const string& _msg, const string& _file, int _line, const string& _function )
{
	this->log( ENUM_LOG_LEVEL::WARNING, _msg, _file, _line, _function );
}
static bool nag_flag = true;

void LOGGER::log( const ENUM_LOG_LEVEL& _level, const string& _msg, const string& _file, int _line, const string& _function )
{
	LOG_CONFIGURATOR* root_logger = LOG_CONFIGURATOR::get_root_configurator();

	if( this->level == ENUM_LOG_LEVEL::INVALID )
	{
		if( root_logger == nullptr )
		{
			// Do nothing.  If root logger is not configured dump everything.
			if( nag_flag )
			{
				cerr << "LOGGER:: Root configurator is not present.  Please fix." << endl;
				nag_flag = false;
			}
		}
		else
		{
			/**
			 * This doesn't make sense.  Why are we comparing our level to root logger?  Our level has been determined to be invalid.
			 */
			if( _level < root_logger->get_level() )
			{
				return;
			}
		}
	}
	else
	{
		if( _level < this->level )
		{
			return;
		}
	}

	/*
	 * What's going on here?  So if a root logger does exist we just dump to it?  This behaviour is contrary to the above block.
	 */
	if( root_logger != nullptr )
	{
		root_logger->log( this->name, _level, _msg, _file, _line, _function );
	}
	else
	{
		cerr << get_iso_date_time() << " - [" << LEVEL_NAMES[static_cast<unsigned int> ( _level )] << "] " << this->name << ":" << _file << "@" << _line << ":" << _function << " -- " << _msg << endl;
	}

	return;
}
void LOGGER::configure( const string& _name, const ENUM_LOG_LEVEL& _level )
{
	this->name = _name;
	this->level = _level;
}

