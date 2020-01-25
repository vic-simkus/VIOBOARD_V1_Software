#include "lib/logger.hpp"
#include "lib/log_configurator.hpp"

using namespace BBB_HVAC::LOGGING;

LOG_CONFIGURATOR* LOG_CONFIGURATOR::root_configurator = nullptr;

#include <iostream>
#include <ostream>
#include <sstream>

#include <unistd.h>
namespace BBB_HVAC
{
	namespace LOGGING
	{

		const string LEVEL_NAMES[] = { "INVALID", "TRACE", "DEBUG", "INFO", "WARNING", "ERROR" };
	}
}


LOG_CONFIGURATOR::LOG_CONFIGURATOR( int _fd, ENUM_LOG_LEVEL _level ) : TPROTECT_BASE( "LOG_CONFIGURATOR" )
{
	this->level = _level;
	this->fd = _fd;
	this->had_error = false;

	//string msg = "LOG_CONFIGURATOR instantiated. ";
	//write( this->fd, msg.data(), msg.length() );

	if ( LOG_CONFIGURATOR::root_configurator == nullptr )
	{
		//msg = "This LOG_CONFIGURATOR instance is root.\n";
		//write( this->fd, msg.data(), msg.length() );

		LOG_CONFIGURATOR::root_configurator = this;
	}

	return;
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
bool LOG_CONFIGURATOR::is_had_error( void ) const
{
	return this->had_error;
}
void LOG_CONFIGURATOR::log( const string& _log_name, const ENUM_LOG_LEVEL& _level, const string& _msg, const string& _file, int _line, const string& _function )
{
	if ( _level < this->level )
	{
		return;
	}

	/*
	We do all the time intensive stuff before we obtain the lock.
	*/

	stringstream output_buffer;
	output_buffer << get_iso_date_time() << " - [" << LEVEL_NAMES[static_cast<unsigned int> ( _level )] << "] " << _log_name << ":" << _file << "@" << _line << ":" << _function << " -- " << _msg << endl;
	string str = output_buffer.str();

	this->obtain_lock_ex();

	try
	{
		if ( write( this->fd, str.data(), str.length() ) < 0 )
		{
			this->had_error = true;
		}
	}
	catch ( ... )
	{
		//ignore
		this->had_error = true;
	}

	this->release_lock();

	return;
}