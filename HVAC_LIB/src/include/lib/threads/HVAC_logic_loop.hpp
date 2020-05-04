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

		enum class OPERATING_MODE : unsigned char
		{
			NONE,
			DELAY_ON,
			OPERATING,
			DELAY_OFF
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

		class AI_VALUE
		{
			public:
				AI_VALUE( float _min, float _max ) {
					this->__init();
					this->setRange( _min, _max );

					this->valid = false;
					this->value = 0;

					return;
				}

				AI_VALUE() {
					this->__init();

					return;
				}

				~AI_VALUE() {
					this->__init();

					return;
				}

				void setRange( float _min, float _max ) {
					this->min = _min;
					this->max = _max;

					return;
				}

				float getMin( void ) const {
					return min;
				}

				float getMax( void ) const {
					return max;
				}

				bool isValid( void ) const {
					return this->valid;
				}

				void operator=( const float& _v ) {
					this->value = _v;
					this->valid = ( _v >= this->min && _v <= this->max );

					return;
				}

				operator float() const {
					return this->value;
				}

				operator bool() const {
					return this->valid;
				}



			protected:
				float min;
				float max;

				float value;
				bool valid;

				void __init( void ) {
					this->min = 0;
					this->max = 0;
					this->valid = false;
					this->value = 0;
				}
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
				OPERATING_MODE op_mode;

				HEAT_MODE mode_heat;
				COOL_MODE mode_cool;
				COOL_MODE mode_dehum;

				unsigned int heat_mode_clicks;
				unsigned int cool_mode_clicks;
				unsigned int dehum_mode_clicks;

				unsigned int switch_clicks;
				unsigned int mode_clicks;

				bool in_ai_failure;
				unsigned long ai_failure_clicks;

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

						unsigned int sp_ahu_delay_pre_cooling;
						unsigned int sp_ahu_delay_pre_heating;
						unsigned int sp_ahu_delay_post_cooling;
						unsigned int sp_ahu_delay_post_heating;

						unsigned int sp_cooling_setpoint_delay;
						unsigned int sp_heating_setpoint_delay;
						unsigned int sp_dehum_setpoint_delay;

						unsigned int sp_mode_switch_delay;

						float sp_heating_deadband;
						float sp_cooling_deadband;
						float sp_dehum_deadband;

						float sp__temp_input_min;
						float sp__temp_input_max;

						float sp__rh_input_min;
						float sp__rh_input_max;

						AI_VALUE temp_value;
						float rh_value;

						float heating_action_on_point;
						float cooling_action_on_point;

						float heating_action_off_point;
						float cooling_action_off_point;

						float dehum_action_on_point;
						float dehum_action_off_point;

						float dehum_cancel_temp_point;
						float dehum_min_temp_point;
				};

				/**
				Pointer for a decider function.
				A decider function evaluates the current state and returns TRUE if the current action should continue.  If the current action should terminate FALSE is returned.
				*/
				typedef  bool ( BBB_HVAC::HVAC_LOGIC::HVAC_LOGIC_LOOP::*ACTION_DECIDER_PTR )( const HVAC_LOOP_INVOCATION_CONTEXT& );

				/**
				Pointer to a delay decider function.
				A decider function evaluates the current state and return TRUE if the delay should continue.  If the delay should be terminated the function returns FALSE.
				*/
				typedef bool( BBB_HVAC::HVAC_LOGIC::HVAC_LOGIC_LOOP::*DELAY_DECIDER_PTR )( const HVAC_LOOP_INVOCATION_CONTEXT& );

				/**
				Pointer to a mode switcher function.
				*/
				typedef void( BBB_HVAC::HVAC_LOGIC::HVAC_LOGIC_LOOP::*MODE_SWITCHER_PTR )( OPERATING_MODE );

				/**
				Pointer to an output switcher function.  This function is expected to toggle the digital outputs in such a way as appropriate for the  given mode.
				*/
				typedef void( BBB_HVAC::HVAC_LOGIC::HVAC_LOGIC_LOOP::*OUTPUT_SETTER_PTR )( OPERATING_MODE );

			private:

				void process_logic_none( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx );
				void process_logic_do( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx, ACTION_DECIDER_PTR _action_decider, DELAY_DECIDER_PTR _delay_decider, MODE_SWITCHER_PTR _mode_switcher, OUTPUT_SETTER_PTR _output_switcher );

				void switch_op_state( OPERATING_STATE _new_state );

				void mode_switcher_heating( const OPERATING_MODE _new_mode );
				void mode_switcher_cooling( const OPERATING_MODE _new_mode );

				void output_setter_heating( const OPERATING_MODE _mode );
				void output_setter_cooling( const OPERATING_MODE _mode );

				bool action_decider_cooling( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx ) ;
				bool action_decider_heating( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx );
				bool action_decider_dehumidification( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx );

				bool delay_decider_heating( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx );
				bool delay_decider_cooling( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx );
				bool delay_decider_dehumidification( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx );

				bool delay_decider( unsigned int _pre_sp, unsigned int _post_sp, unsigned int _sp ) const;

		};

		/// Point name AHU_HEATER define
#define PN_AHU_HEATER "AHU_HEATER"

		/// Point name AC_COMPRESSOR define
#define PN_AC_COMPRESSOR "AC_COMPRESSOR"

		/// Point name AHU_FAN define
#define PN_AHU_FAN "AHU_FAN"

#define SP_SPACE_TEMP 					"SPACE TEMP"
#define SP_SPACE_RH						"SPACE RH"

#define SP_SPACE_TEMP_DELTA_HIGH		"SPACE TEMP DELTA HIGH"
#define SP_SPACE_TEMP_DELTA_LOW			"SPACE TEMP DELTA LOW"
#define SP_SPACE_RH_DELTA 				"SPACE RH DELTA"

#define SP_AHU_FAN_DELAY_PRE_COOLING	"AHU FAN DELAY PRE COOLING"
#define SP_AHU_FAN_DELAY_POST_COOLING	"AHU FAN DELAY POST COOLING"
#define SP_AHU_FAN_DELAY_PRE_HEATING	"AHU FAN DELAY PRE HEATING"
#define SP_AHU_FAN_DELAY_POST_HEATING	"AHU FAN DELAY POST HEATING"

#define SP_MODE_SWITCH_DELAY 			"MODE SWITCH DELAY"

#define SP_COOLING_DEADBAND 			"COOLING DEADBAND"
#define SP_HEATING_DEADBAND 			"HEATING DEADBAND"
#define SP_SPACE_RH_DEADBAND			"DEHUM DEADBAND"

#define SP_COOLING_SETPOINT_DELAY 		"COOLING SETPOINT DELAY"
#define SP_HEATING_SETPOINT_DELAY 		"HEATING SETPOINT DELAY"
#define SP_DEHUM_SETPOINT_DELAY 		"DEHUM SETPOINT DELAY"

#define SP_SPACE_RH_TEMP_DELTA			"SPACE RH TEMP DELTA"

#define SP__TEMP_INPUT_MIN				"_TEMP_INPUT_MIN"
#define SP__TEMP_INPUT_MAX				"_TEMP_INPUT_MAX"

#define SP__TEMP_INPUT_MIN				"_TEMP_INPUT_MIN"
#define SP__TEMP_INPUT_MAX				"_TEMP_INPUT_MAX"

#define SP__RH_INPUT_MIN				"_RH_INPUT_MIN"
#define SP__RH_INPUT_MAX				"_RH_INPUT_MAX"

#define AI_SPACE_1_TEMP 				"SPACE_1_TEMP"
#define AI_SPACE_1_RH 					"SPACE_1_RH"

	}
}

#endif /* SRC_INCLUDE_LIB_HVAC_LOGIC_LOOP_HPP_ */
