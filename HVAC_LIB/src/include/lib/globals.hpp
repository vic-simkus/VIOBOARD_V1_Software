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

#ifndef SRC_INCLUDE_LIB_GLOBALS_HPP_
#define SRC_INCLUDE_LIB_GLOBALS_HPP_

#include <pthread.h>

namespace BBB_HVAC
{
	class COMMAND_LINE_PARMS;

	/**
	 * Forward definition
	 */
	class LOGIC_PROCESSOR_BASE;
	class WATCHDOG;

	namespace IOCOMM
	{
		class SER_IO_COMM;
	}

	namespace LOGGING
	{
		class LOG_CONFIGURATOR;
		enum class ENUM_LOG_LEVEL : unsigned int;
		}

		/**
		 * Namespace containing all global (to the program) variables.
		 */
		namespace GLOBALS
	{

		/**
		 * Global exit flag.
		 * This flag is more or less a vestige.  The main loop does break on this and some threads do set this to true,
		 * but no threads use this in their even loop.
		 */
		extern bool global_exit_flag;

		/**
		 * Reset by the logic_loop thread.
		 */
		extern WATCHDOG* watchdog;

		/**
		 * Instance of the logic processor class.
		 */
		extern LOGIC_PROCESSOR_BASE* logic_instance;
		//extern IOCOMM::SER_IO_COMM * io_instance;

		extern LOGGING::LOG_CONFIGURATOR* root_log_configurator;

		extern void configure_logging( int _fd, const LOGGING::ENUM_LOG_LEVEL&  _level );

		extern void configure_watchdog();
		extern void destroy_watchdog();

		extern void configure_signals( void );


		extern int create_logger_fd( const COMMAND_LINE_PARMS& _clp, bool _test );
		extern void drop_privs( void );
		extern int check_privs( void );
		extern void daemon_self( void );


	}
}

#endif /* SRC_INCLUDE_LIB_GLOBALS_HPP_ */
