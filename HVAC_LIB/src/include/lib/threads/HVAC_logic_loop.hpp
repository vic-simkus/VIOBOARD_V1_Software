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

#ifndef SRC_INCLUDE_LIB_HVAC_LOGIC_LOOP_HPP_
#define SRC_INCLUDE_LIB_HVAC_LOGIC_LOOP_HPP_

#include "lib/threads/logic_thread.hpp"
#include "lib/logger.hpp"

#include <string>

using namespace std;

namespace BBB_HVAC
{

	namespace HVAC_LOGIC
	{
		enum class OPERATING_STATE : unsigned char
		{
			NONE = 0,
			HEATING,
			COOLING
		};

		/**
		 * Class for controlling a basic home HVAC system.
		 * Control consists of following outputs:
		 * 1 - AC compressor on/off
		 * 2 - Air handler fan on/off
		 * 3 - Heater on/off
		 *
		 * Input consists of the following:
		 * 1 - Space temperature probe (4-20mA)
		 * 2 - Space humidity probe (4-20mA)
		 * 3 - Supply air temperature (ICTD)
		 */
		class HVAC_LOGIC_LOOP : public LOGIC_PROCESSOR_BASE
		{
		public:

			/**
			 * Constructor
			 */
			HVAC_LOGIC_LOOP( CONFIGURATOR* _config );

			/**
			 * Destructor
			 */
			virtual ~HVAC_LOGIC_LOOP();

			/**
			 * Method that is called to perform in-time processing.
			 */
			void process_logic( void ) throw( exception );

			void pre_process( void ) throw( exception );

			void post_process( void ) throw( exception );


		protected:
			/**
			 * Logger
			 */
			DEF_LOGGER;
		private:
		};
	}
}

#endif /* SRC_INCLUDE_LIB_HVAC_LOGIC_LOOP_HPP_ */
