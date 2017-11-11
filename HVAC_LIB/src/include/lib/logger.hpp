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
#include "lib/tprotect_base.hpp"

#define DEF_LOGGER BBB_HVAC::LOGGING::LOGGER __logger__

#define DEF_LOGGER_STAT(name) static BBB_HVAC::LOGGING::LOGGER logger(name)

#define INIT_LOGGER(name) this->__logger__.configure(name)
#define INIT_LOGGER_P(name) this->logger->configure(name)

#define LOG_TRACE_INST(inst,message) inst->__logger__.log_trace(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_DEBUG_INST(inst,message) inst->__logger__.log_debug(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_INFO_INST(inst,message) inst->__logger__.log_info(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_ERROR_INST(inst,message) inst->__logger__.log_error(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);

#define LOG_TRACE_STAT(message) logger.log_trace(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_DEBUG_STAT(message) logger.log_debug(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_INFO_STAT(message) logger.log_info(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_ERROR_STAT(message) logger.log_error(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);

#define LOG_TRACE(message) this->__logger__.log_trace(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_DEBUG(message) this->__logger__.log_debug(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_INFO(message) this->__logger__.log_info(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_WARNING(message) this->__logger__.log_warning(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_ERROR(message) this->__logger__.log_error(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);

#define LOG_TRACE_P(message) this->logger->log_trace(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_DEBUG_P(message) this->logger->log_debug(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_INFO_P(message) this->logger->log_info(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_WARNING_P(message) this->logger->log_warning(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LOG_ERROR_P(message) this->logger->log_error(message,__FILE__,__LINE__,__PRETTY_FUNCTION__);

using namespace std;

namespace BBB_HVAC
{
	namespace LOGGING
	{
		class LOG_CONFIGURATOR : public TPROTECT_BASE
		{
		public:
			LOG_CONFIGURATOR(ENUM_LOG_LEVEL _level);
			~LOG_CONFIGURATOR();

			ENUM_LOG_LEVEL get_level(void) const;

			static LOG_CONFIGURATOR* get_root_configurator(void);
			static void destroy_root_configurator(void);

			void log(const string& _log_name,const ENUM_LOG_LEVEL& _level, const string& _msg, const string& _file, int _line, const string& _function);

		protected:
			ENUM_LOG_LEVEL level;
			static LOG_CONFIGURATOR* root_configurator;

			stringstream output_buffer;

		};
		class LOGGER
		{
		public:
			LOGGER();
			LOGGER(const string& _name);
			void log_trace(const string& _msg, const string& _file, int _line, const string& _function);
			void log_debug(const string& _msg, const string& _file, int _line, const string& _function);
			void log_info(const string& _msg, const string& _file, int _line, const string& _function);
			void log_warning(const string& _msg, const string& _file, int _line, const string& _function);
			void log_error(const string& _msg, const string& _file, int _line, const string& _function);

			void log(const ENUM_LOG_LEVEL& _level, const string& _msg, const string& _file, int _line, const string& _function);
			void configure(const string& _name, const ENUM_LOG_LEVEL& _level = ENUM_LOG_LEVEL::INVALID);

		protected:
			string name;
			ENUM_LOG_LEVEL level;
		private:
		};
	}
}

#endif /* SRC_INCLUDE_LIB_LOGGER_HPP_ */
