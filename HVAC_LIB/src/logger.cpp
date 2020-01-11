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
#include "lib/log_configurator.hpp"
#include <iostream>
#include <ostream>
#include <sstream>

#include <unistd.h>


using namespace BBB_HVAC::LOGGING;

LOGGER::LOGGER( const string& _name, ENUM_LOG_LEVEL _level )
{
	this->name = _name;
	this->level = _level;
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
	LOG_CONFIGURATOR* log_configurator = LOG_CONFIGURATOR::get_root_configurator();

	if ( log_configurator == nullptr )
	{
		// Logging system not configured.
		//
		if ( nag_flag )
		{
			cerr << "LOGGER:: Root configurator is not present. No further log output will be produced." << endl;
			nag_flag = false;
		}

		return;
	}

	/*
	If we don't have a level set we steal it from the log_configurator.
	*/
	if ( this->level == ENUM_LOG_LEVEL::INVALID )
	{
		this->level = log_configurator->get_level();
	}

	/*
	If the supplied log level is lower than our log level ignore the log message.
	*/
	if ( _level < this->level )
	{
		return;
	}


	if ( log_configurator != nullptr )
	{
		log_configurator->log( this->name, _level, _msg, _file, _line, _function );
	}

	return;
}
/*
void LOGGER::configure( const string& _name, const ENUM_LOG_LEVEL& _level )
{
	this->name = _name;
	this->level = _level;
}

*/