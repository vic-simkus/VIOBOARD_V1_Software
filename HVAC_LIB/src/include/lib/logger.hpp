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

#ifndef SRC_INCLUDE_LIB_LOGGER_HPP_
#define SRC_INCLUDE_LIB_LOGGER_HPP_

#include <string>
#include <sstream>

#include "lib/hvac_types.hpp"

#define DEF_LOGGER BBB_HVAC::LOGGING::LOGGER_PTR __logger__

#define DEF_LOGGER_STAT(name) static BBB_HVAC::LOGGING::LOGGER_PTR __logger__(new BBB_HVAC::LOGGING::LOGGER(name,BBB_HVAC::LOGGING::ENUM_LOG_LEVEL::TRACE))

#define INIT_LOGGER(name) this->__logger__.reset(new BBB_HVAC::LOGGING::LOGGER(name,BBB_HVAC::LOGGING::ENUM_LOG_LEVEL::TRACE));

#define LOG_TRACE(message) __logger__->log_trace(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_DEBUG(message) __logger__->log_debug(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_INFO(message) __logger__->log_info(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_WARNING(message) __logger__->log_warning(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_ERROR(message) __logger__->log_error(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);

/*
#define LOG_TRACE_P(message) this->logger->log_trace(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_DEBUG_P(message) this->logger->log_debug(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_INFO_P(message) this->logger->log_info(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_WARNING_P(message) this->logger->log_warning(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_ERROR_P(message) this->logger->log_error(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
*/

using namespace std;

namespace BBB_HVAC
{
	namespace LOGGING
	{

		enum class ENUM_LOG_LEVEL
			: unsigned int
		{
			INVALID = 0, /// Invalid level.  Parent's level will be utilized
			TRACE,		/// Trace-level logging level
			DEBUG,		/// Debug-level logging level
			INFO,		/// Information-level logging level
			WARNING,	/// Warning-level logging level
			ERROR		/// Error-level logging level
		} ;


		extern const string LEVEL_NAMES[];
		/**
		 * Logging levels
		 */

		/**
		 * Helper operator to turn a logging level into a string number
		 * \param os Output stream
		 * \param _v Logging level instance
		 * \return  Output stream
		 */
		inline std::ostream& operator<< ( std::ostream& os, ENUM_LOG_LEVEL _v )
		{
			return os << static_cast < unsigned int >( _v );
		}

		class LOGGER
		{
			public:
				/**
				Constructor.

				\param _name	Name of the logger.
				\param _level	Logger's level.  The level of a logger acts as a filter.  No messages with levels bellow the logger instance level will be emitted.
				*/
				LOGGER( const string& _name, ENUM_LOG_LEVEL _level = ENUM_LOG_LEVEL::ERROR );
				void log_trace( const string& _msg, const string& _file, int _line, const string& _function );
				void log_debug( const string& _msg, const string& _file, int _line, const string& _function );
				void log_info( const string& _msg, const string& _file, int _line, const string& _function );
				void log_warning( const string& _msg, const string& _file, int _line, const string& _function );
				void log_error( const string& _msg, const string& _file, int _line, const string& _function );

				void log( const ENUM_LOG_LEVEL& _level, const string& _msg, const string& _file, int _line, const string& _function );
				void configure( const string& _name, const ENUM_LOG_LEVEL& _level = ENUM_LOG_LEVEL::ERROR );

			protected:
				string name;
				ENUM_LOG_LEVEL level;
			private:
				LOGGER();
		};
	}
}

#endif /* SRC_INCLUDE_LIB_LOGGER_HPP_ */
