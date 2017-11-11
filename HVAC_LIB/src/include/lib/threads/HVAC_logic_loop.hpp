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
		 * Output pin mapping
		 */
		enum class D_OUTPUT_PIN_ENUM : unsigned int
		{
			AC_COMP = 0, /// Digital output for turning the AC condenser package on/off
			AIR_HANDLER, /// Digital output for turning the air handler fan on/off
			HEATER, /// Digital output for turning the heater on/of
			__COUNT /// Number of defined pins/

		};

		/**
		 * Analog input pin mapping
		 */
		enum class A_INPUT_PIN_ENUM : unsigned int
		{
			SPACE_TEMP = 0, /// Analog input for space temp
			SPACE_RH, /// Analog input for space relative humidity
			SUPPLY_TEMP, /// Analog input for air handler supply temperature
			__COUNT
		};

		/**
		 * Constructor
		 */
		HVAC_LOGIC_LOOP(CONFIGURATOR* _config);

		/**
		 * Destructor
		 */
		virtual ~HVAC_LOGIC_LOOP();

		/**
		 * Method that is called to perform in-time processing.
		 */
		void process_logic(void) throw(exception);

		void pre_process(void) throw(exception);

		void post_process(void) throw(exception);


	protected:

		string do_0_ac_path;
		string do_1_air_handler_path;
		string do_2_heater_path;

		string ai_0_space_temp_path;
		string ai_1_space_rh_path;
		string ai_2_supply_temp_path;

		/**
		 * Logger
		 */
		DEF_LOGGER;
	private:
	};
}

#endif /* SRC_INCLUDE_LIB_HVAC_LOGIC_LOOP_HPP_ */
