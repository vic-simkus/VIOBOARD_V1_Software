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
			COOLING,
			DEHUMIDIFYING
		};

		enum class HEAT_MODE : unsigned char
		{
			NONE = 0,
			DELAY_ON,
			HEATING,
			DELAY_OFF
		};

		enum class COOL_MODE : unsigned char
		{
			NONE = 0,
			DELAY_ON,
			COOLING,
			DELAY_OFF
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
				OPERATING_STATE op_state;

				HEAT_MODE mode_heat;
				COOL_MODE mode_cool;
				COOL_MODE mode_dehum;

				unsigned int heat_mode_clicks;
				unsigned int cool_mode_clicks;
				unsigned int dehum_mode_clicks;

				unsigned int switch_clicks;

				class HVAC_LOOP_INVOCATION_CONTEXT
				{
					public:
						HVAC_LOOP_INVOCATION_CONTEXT( HVAC_LOGIC_LOOP* _parent );

						float sp_space_temp;
						float sp_space_rh;
						float sp_space_temp_d_high;
						float sp_space_temp_d_low;
						float sp_space_rh_d;
						float sp_space_rh_temp_d;

						unsigned int sp_ahu_delay_cooling;
						unsigned int sp_ahu_delay_heating;

						unsigned int sp_cooling_setpoint_delay;
						unsigned int sp_heating_setpoint_delay;
						unsigned int sp_dehum_setpoint_delay;

						unsigned int sp_mode_switch_delay;
						unsigned int sp_heating_deadband;
						unsigned int sp_cooling_deadband;
						unsigned int sp_dehum_deadband;

						float temp_value;
						float rh_value;

						float heating_point;
						float cooling_point;

						float dehum_action_point;
						float dehum_cancel_temp_point;
				};
			private:

				void switch_op_state( OPERATING_STATE _new_state );
				void switch_heating_mode( HEAT_MODE _new_mode );
				void switch_cooling_mode( COOL_MODE _new_mode );
				void switch_dehumidification_mode( COOL_MODE _new_mode );

				void process_logic_none( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx );
				void process_logic_cooling( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx );
				void process_logic_heating( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx );
				void process_logic_dehumidification( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx );

				void set_outputs_for_heating( const HEAT_MODE _mode );
				void set_outputs_for_cooling( const COOL_MODE _mode );
				void set_outputs_for_dehumidification( const COOL_MODE _mode );
		};

		/// Point name AHU_HEATER define
#define PN_AHU_HEATER "AHU_HEATER"

		/// Point name AC_COMPRESSOR define
#define PN_AC_COMPRESSOR "AC_COMPRESSOR"

		/// Point name AHU_FAN define
#define PN_AHU_FAN "AHU_FAN"

#define SP_SPACE_TEMP 				"SPACE TEMP"
#define SP_SPACE_RH					"SPACE RH"

#define SP_SPACE_TEMP_DELTA_HIGH	"SPACE TEMP DELTA HIGH"
#define SP_SPACE_TEMP_DELTA_LOW		"SPACE TEMP DELTA LOW"
#define SP_SPACE_RH_DELTA 			"SPACE RH DELTA"

#define SP_AHU_FAN_DELAY_COOLING	"AHU FAN DELAY COOLING"
#define SP_AHU_FAN_DELAY_HEATING	"AHU FAN DELAY HEATING"
#define SP_MODE_SWITCH_DELAY 		"MODE SWITCH DELAY"

#define SP_COOLING_DEADBAND 		"COOLING DEADBAND"
#define SP_HEATING_DEADBAND 		"HEATING DEADBAND"
#define SP_SPACE_RH_DEADBAND		"DEHUM DEADBAND"

#define SP_COOLING_SETPOINT_DELAY 	"COOLING SETPOINT DELAY"
#define SP_HEATING_SETPOINT_DELAY 	"HEATING SETPOINT DELAY"
#define SP_DEHUM_SETPOINT_DELAY 	"DEHUM SETPOINT DELAY"

#define SP_SPACE_RH_TEMP_DELTA		"SPACE RH TEMP DELTA"

#define AI_SPACE_1_TEMP 			"SPACE_1_TEMP"
#define AI_SPACE_1_RH 				"SPACE_1_RH"

	}
}

#endif /* SRC_INCLUDE_LIB_HVAC_LOGIC_LOOP_HPP_ */
