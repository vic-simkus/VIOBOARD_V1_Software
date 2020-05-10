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


#include "lib/threads/HVAC_logic_loop.hpp"
#include "lib/configurator.hpp"
#include <math.h>
#include <exception>
using namespace BBB_HVAC;
using namespace BBB_HVAC::HVAC_LOGIC;

#include <iostream>


HVAC_LOGIC_LOOP::HVAC_LOOP_INVOCATION_CONTEXT::HVAC_LOOP_INVOCATION_CONTEXT( HVAC_LOGIC_LOOP* _parent )
{
	/*
	Space temperature set point
	*/
	this->sp_space_temp = ( float )_parent->get_sp_value( SP_SPACE_TEMP );

	/*
	Space humidity set point
	*/
	this->sp_space_rh = ( float )_parent->get_sp_value( SP_SPACE_RH );

	/*
	Delta T relative to SP above which the system will switch into cooling mode.
	*/
	this->sp_space_temp_d_high = ( float )_parent->get_sp_value( SP_SPACE_TEMP_DELTA_HIGH );

	/*
	Delta T relative to SP bellow which the system will switch into heating mode.
	*/
	this->sp_space_temp_d_low = ( float )_parent->get_sp_value( SP_SPACE_TEMP_DELTA_LOW );

	/*
	Delta percent relative to the SP above which the system will switch into dehumidification mode.
	*/
	this->sp_space_rh_temp_d = ( float )_parent->get_sp_value( SP_SPACE_RH_TEMP_DELTA );

	/*
	Delay before the AHU fan is switched on/off after the condenser is switched on/off.
	*/
	this->sp_ahu_delay_pre_cooling = ( unsigned int )_parent->get_sp_value( SP_AHU_FAN_DELAY_PRE_COOLING );
	this->sp_ahu_delay_post_cooling = ( unsigned int )_parent->get_sp_value( SP_AHU_FAN_DELAY_POST_COOLING );

	/*
	Delay before the AHU fan is switch on/off after the heating coils are turned on/off.
	*/
	this->sp_ahu_delay_pre_heating = ( unsigned int )_parent->get_sp_value( SP_AHU_FAN_DELAY_PRE_HEATING );
	this->sp_ahu_delay_post_heating = ( unsigned int )_parent->get_sp_value( SP_AHU_FAN_DELAY_POST_HEATING );

	/*
	Delay between any mode switches
	*/
	this->sp_mode_switch_delay = ( unsigned int )_parent->get_sp_value( SP_MODE_SWITCH_DELAY );

	/*
	Heating dead band.  System will shut off heating when the temperature reaches SP + HEATING_DEADBAND for a certain amount of time.
	*/
	this->sp_heating_deadband = ( float )_parent->get_sp_value( SP_HEATING_DEADBAND );

	/*
	Cooling dead band.  System will shut off cooling when the temperature reaches SP - COOLING_DEADBAND for a certain amount of time.
	*/
	this->sp_cooling_deadband = ( float )_parent->get_sp_value( SP_COOLING_DEADBAND );

	/*
	Dehumidification dead band.  System will shut off dehumidification when the temperature reaches SP - DEHUM_DEADBAND for a certain amount of time.
	*/
	this->sp_dehum_deadband = ( float )_parent->get_sp_value( SP_SPACE_RH_DEADBAND );

	/*
	Delay before cooling mode will disengage after temperature requirements met.
	*/
	this->sp_cooling_setpoint_delay = ( unsigned int )_parent->get_sp_value( SP_COOLING_SETPOINT_DELAY );

	/*
	Delay before heating mode will disengage after the temperature requirements are met.
	*/
	this->sp_heating_setpoint_delay = ( unsigned int )_parent->get_sp_value( SP_HEATING_SETPOINT_DELAY );

	/*
	Delay before dehumidification mode will disengage after the humidity requirements are met.
	*/
	this->sp_dehum_setpoint_delay = ( unsigned int )_parent->get_sp_value( SP_DEHUM_SETPOINT_DELAY );

	this->sp_space_rh_d = ( float )_parent->get_sp_value( SP_SPACE_RH_DELTA );

	this->sp__temp_input_min = ( float )_parent->get_sp_value( SP__TEMP_INPUT_MIN );
	this->sp__temp_input_max = ( float )_parent->get_sp_value( SP__TEMP_INPUT_MAX );

	this->sp__rh_input_min = ( float )_parent->get_sp_value( SP__RH_INPUT_MIN );
	this->sp__rh_input_max = ( float )_parent->get_sp_value( SP__RH_INPUT_MAX );

	/*
	Calculated values
	*/

	/*
	Get the temperature and relative humidity value and round it off to one decimal place.
	*/

	this->temp_value = AI_VALUE( this->sp__temp_input_min, this->sp__temp_input_max );

	//float v = float( int( ( _parent->get_ai_value( AI_SPACE_1_TEMP ) * 10 ) ) ) / 10;
	//std::cout << "Temp: " + num_to_str( v )  << std::endl;

	this->temp_value = float( int( ( _parent->get_ai_value( AI_SPACE_1_TEMP ) * 10 ) ) ) / 10;

	//std::cout << "FTemp: " + num_to_str( ( float )this->temp_value )  << "[" << this->temp_value.getMin() << "," << this->temp_value.getMax() << "]" << std::endl;

	this->rh_value = AI_VALUE( this->sp__rh_input_min, this->sp__rh_input_max );
	this->rh_value = float( int( ( _parent->get_ai_value( AI_SPACE_1_RH ) * 10 ) ) ) / 10;

	/*
	The space temp that the system will initiate switch to heating mode.
	SP SPACE TEMP DELTA LOW is expected to be specified in negative units.
	*/
	this->heating_action_on_point = ( sp_space_temp + sp_space_temp_d_low );
	this->heating_action_off_point = ( this->heating_action_on_point + this->sp_heating_deadband );

	/*
	The space temp that the system will initiate switch to cooling mode.
	SP SPACE TEMP DELTA HIGH is expected to be specified in positive units.
	*/
	this->cooling_action_on_point = ( sp_space_temp + sp_space_temp_d_high );
	this->cooling_action_off_point = ( this->cooling_action_on_point - this->sp_cooling_deadband );

	/*
	The space RH that the system will initiate switch to dehumidification mode.
	*/
	this->dehum_action_on_point = sp_space_rh + sp_space_rh_d;
	this->dehum_action_off_point = sp_space_rh - this->sp_dehum_deadband;

	/*
	The space temp that the system will cancel dehumidification mode.
	*/
	this->dehum_cancel_temp_point = sp_space_temp - sp_space_rh_temp_d;

	/*
	Minimum temperature needed to initiate dehumidification
	*/
	this->dehum_min_temp_point = ( this->dehum_cancel_temp_point + ( this->sp_space_rh_temp_d / 2 ) );

	return;
}
void HVAC_LOGIC_LOOP::process_logic( void )
{
	/*
	 * We do not touch the mutex here.  The thread loop takes care of that
	 * for us before calling process_logic
	 */

	// This is purely for infotainment value.  We don't use this anywhere in logic
	this->logic_status_core.iterations += 1;

	/*
	Creates a context; pre-calculates values and such.
	We do want to keep this dynamic because we want the various setpoints to be dynamic.
	*/

	const HVAC_LOOP_INVOCATION_CONTEXT ictx( this );

	if ( !ictx.temp_value )
	{
		// Temperature input has failed.

		if ( !this->in_ai_failure )
		{
			LOG_ERROR( "Main temperature input has failed." );
			this->in_ai_failure = true;
			this->switch_op_state( OPERATING_STATE::NONE );
		}
		else
		{
			this->ai_failure_clicks += 1;
		}
	}
	else
	{
		this->in_ai_failure = false;
		this->ai_failure_clicks = 0;

		switch ( this->op_state )
		{
			case OPERATING_STATE::NONE:
				this->process_logic_none( ictx );
				break;

			case OPERATING_STATE::HEATING:
				this->process_logic_do( ictx, &HVAC_LOGIC_LOOP::action_decider_heating, &HVAC_LOGIC_LOOP::delay_decider_heating , &HVAC_LOGIC_LOOP::mode_switcher_heating , &HVAC_LOGIC_LOOP::output_setter_heating );
				break;

			case OPERATING_STATE::COOLING:
				this->process_logic_do( ictx, &HVAC_LOGIC_LOOP::action_decider_cooling, &HVAC_LOGIC_LOOP::delay_decider_cooling , &HVAC_LOGIC_LOOP::mode_switcher_cooling , &HVAC_LOGIC_LOOP::output_setter_cooling );
				break;

			case OPERATING_STATE::DEHUMIDIFYING:
				this->process_logic_do( ictx, &HVAC_LOGIC_LOOP::action_decider_dehumidification, &HVAC_LOGIC_LOOP::delay_decider_cooling , &HVAC_LOGIC_LOOP::mode_switcher_cooling , &HVAC_LOGIC_LOOP::output_setter_cooling );
				break;
		};
	}

	return;
}

void HVAC_LOGIC_LOOP::process_logic_none( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx )
{
	/*
	There is a delay between switching modes.  If nothing else it's a safety to keep the system
	from flip flopping in case of ... whatever.
	*/
	if ( this->switch_clicks >= _ctx.sp_mode_switch_delay )
	{
		if ( ( ( float )_ctx.temp_value ) >= _ctx.cooling_action_on_point )
		{
			LOG_DEBUG( "Space temp " + num_to_str( ( float )_ctx.temp_value ) + " >= cooling action point: " + num_to_str( _ctx.cooling_action_on_point ) + ".  Switching to cooling mode." );
			this->switch_op_state( OPERATING_STATE::COOLING );
		}
		else if ( ( ( float )_ctx.temp_value ) <= _ctx.heating_action_on_point )
		{
			LOG_DEBUG( "Space temp " + num_to_str( ( float ) _ctx.temp_value ) + " <= heating action point: " + num_to_str( _ctx.heating_action_on_point ) + ".  Switching to heating mode." );
			this->switch_op_state( OPERATING_STATE::HEATING );
		}
		else if ( _ctx.rh_value > _ctx.dehum_action_on_point )
		{
			if ( ( ( float )_ctx.temp_value ) > _ctx.dehum_min_temp_point )
			{
				LOG_DEBUG( "Space RH " + num_to_str( _ctx.rh_value ) + " >= dehumidification action point: " + num_to_str( _ctx.dehum_action_on_point ) + " and space temp: " + num_to_str( ( float )_ctx.temp_value ) + " > " + num_to_str( _ctx.dehum_min_temp_point ) + ".  Switching to dehumidification mode." );
				this->switch_op_state( OPERATING_STATE::DEHUMIDIFYING );
			}
			else
			{
				//LOG_DEBUG( "Dehum not initiated: space temp: " + num_to_str( _ctx.temp_value ) + " < " + num_to_str( _ctx.dehum_min_temp_point ) );
			}
		}
	}
	else
	{
		this->switch_clicks += 1;
	}

	return;
}
void HVAC_LOGIC_LOOP::process_logic_do( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx, ACTION_DECIDER_PTR _action_decider, DELAY_DECIDER_PTR _delay_decider, MODE_SWITCHER_PTR _mode_switcher, OUTPUT_SETTER_PTR _output_setter )
{
	// Set the digital outputs for this particular mode.
	// There is one click delay between setting the mode and the outputs changing.

	( this->*_output_setter )( this->op_mode );

	switch ( this->op_mode )
	{
		case OPERATING_MODE::NONE:
			( this->*_mode_switcher )( OPERATING_MODE::DELAY_ON );
			break;

		case OPERATING_MODE::DELAY_ON:
			this->mode_clicks += 1;

			if ( !( this->*_delay_decider )( _ctx ) )
			{
				( this->*_mode_switcher )( OPERATING_MODE::OPERATING );
			}

			break;

		case OPERATING_MODE::OPERATING:
			if ( ( this->*_action_decider )( _ctx ) )
			{
				// Decider returned TRUE which means we continue operating
				this->mode_clicks = 0;
			}
			else
			{
				// Decider returned FALSE which means that we start the countdown for
				// getting out of this operating mode.
				this->mode_clicks += 1;

				if ( !( this->*_delay_decider )( _ctx ) )
				{
					// The delay decider return FALSE which means that the we should not continue delaying
					( this->*_mode_switcher )( OPERATING_MODE::DELAY_OFF );
				}
			}

			break;

		case OPERATING_MODE::DELAY_OFF:
			this->mode_clicks += 1;

			if ( !( this->*_delay_decider )( _ctx ) )
			{
				// The delay decider return FALSE which means that the we should not continue delaying
				( this->*_mode_switcher )( OPERATING_MODE::NONE );
				this->switch_op_state( OPERATING_STATE::NONE );
			}

			break;
	}
}
bool HVAC_LOGIC_LOOP::action_decider_cooling( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx )
{
	if ( ( ( float )_ctx.temp_value ) >= _ctx.cooling_action_off_point )
	{
		return true;
	}
	else
	{
		LOG_DEBUG( "Cooling stop: space temp: " + num_to_str( ( float )_ctx.temp_value ) + " < " + num_to_str( _ctx.cooling_action_off_point ) );
		return false;
	}
}
bool HVAC_LOGIC_LOOP::action_decider_heating( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx )
{
	if ( ( ( float )_ctx.temp_value ) < _ctx.heating_action_off_point )
	{
		return true;
	}
	else
	{
		return false;
	}

}
bool HVAC_LOGIC_LOOP::action_decider_dehumidification( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx )
{

	if ( ( ( float )_ctx.temp_value ) >= _ctx.dehum_cancel_temp_point )
	{
		// Space temperature is greater than the general set point minus the cooling dead band.
		if ( _ctx.rh_value < _ctx.dehum_action_off_point )
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		// It got too cold.  Abort dehumidification
		LOG_DEBUG( "Aborting dehum.  Space temp: " + num_to_str( ( float )_ctx.temp_value ) + " < " + num_to_str( _ctx.dehum_cancel_temp_point ) );
		return false;
	}
}

bool HVAC_LOGIC_LOOP::delay_decider( unsigned int _pre_sp, unsigned int _post_sp, unsigned int _sp ) const
{
	if ( this->op_mode == OPERATING_MODE::DELAY_ON )
	{
		if ( this->mode_clicks < _pre_sp )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if ( this->op_mode == OPERATING_MODE::DELAY_OFF )
	{
		if ( this->mode_clicks < _post_sp )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if ( this->op_mode == OPERATING_MODE::OPERATING )
	{
		if ( this->mode_clicks > _sp )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		throw std::runtime_error( "Unexpected code path in delay_decider." );
	}
}
bool HVAC_LOGIC_LOOP::delay_decider_heating( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx )
{
	return this->delay_decider( _ctx.sp_ahu_delay_pre_heating, _ctx.sp_ahu_delay_post_heating, _ctx.sp_heating_setpoint_delay );
}
bool HVAC_LOGIC_LOOP::delay_decider_cooling( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx )
{
	return this->delay_decider( _ctx.sp_ahu_delay_pre_cooling, _ctx.sp_ahu_delay_post_cooling, _ctx.sp_cooling_setpoint_delay );
}
bool HVAC_LOGIC_LOOP::delay_decider_dehumidification( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx )
{
	return this->delay_decider( _ctx.sp_ahu_delay_pre_cooling, _ctx.sp_ahu_delay_post_cooling, _ctx.sp_dehum_setpoint_delay );
}

void HVAC_LOGIC_LOOP::output_setter_heating( const OPERATING_MODE _mode )
{
	switch ( _mode )
	{
		case OPERATING_MODE::NONE:
			this->clear_output( PN_AC_COMPRESSOR );
			this->clear_output( PN_AHU_HEATER );
			this->clear_output( PN_AHU_FAN );
			break;

		case OPERATING_MODE::DELAY_ON:
			this->clear_output( PN_AC_COMPRESSOR );
			this->set_output( PN_AHU_HEATER );
			this->clear_output( PN_AHU_FAN );
			break;

		case OPERATING_MODE::OPERATING:
			this->clear_output( PN_AC_COMPRESSOR );
			this->set_output( PN_AHU_HEATER );
			this->set_output( PN_AHU_FAN );
			break;

		case OPERATING_MODE::DELAY_OFF:
			this->clear_output( PN_AC_COMPRESSOR );
			this->clear_output( PN_AHU_HEATER );
			this->set_output( PN_AHU_FAN );
			break;
	};

	return;
}

void HVAC_LOGIC_LOOP::output_setter_cooling( const OPERATING_MODE _mode )
{
	switch ( _mode )
	{
		case OPERATING_MODE::NONE:
			this->clear_output( PN_AC_COMPRESSOR );
			this->clear_output( PN_AHU_HEATER );
			this->clear_output( PN_AHU_FAN );
			break;

		case OPERATING_MODE::DELAY_ON:
			this->set_output( PN_AC_COMPRESSOR );
			this->clear_output( PN_AHU_FAN );
			this->clear_output( PN_AHU_HEATER );
			break;

		case OPERATING_MODE::OPERATING:
			this->set_output( PN_AC_COMPRESSOR );
			this->set_output( PN_AHU_FAN );
			this->clear_output( PN_AHU_HEATER );
			break;

		case OPERATING_MODE::DELAY_OFF:
			this->clear_output( PN_AC_COMPRESSOR );
			this->set_output( PN_AHU_FAN );
			this->clear_output( PN_AHU_HEATER );
			break;
	};

	return;
}

void HVAC_LOGIC_LOOP::mode_switcher_heating( OPERATING_MODE _new_mode )
{
	this->output_setter_heating( _new_mode );
	this->op_mode = _new_mode;
	this->mode_clicks = 0;
	return;
}

void HVAC_LOGIC_LOOP::mode_switcher_cooling( OPERATING_MODE _new_mode )
{
	this->output_setter_cooling( _new_mode );
	this->op_mode = _new_mode;
	this->mode_clicks = 0;
	return;
}

void HVAC_LOGIC_LOOP::switch_op_state( OPERATING_STATE _new_state )
{
	switch ( _new_state )
	{
		case OPERATING_STATE::NONE:
			LOG_DEBUG( "Switching operating state to NONE." );
			break;

		case OPERATING_STATE::HEATING:
			LOG_DEBUG( "Switching operating state to HEATING." );
			break;

		case OPERATING_STATE::COOLING:
			LOG_DEBUG( "Switching operating state to COOLING." );
			break;

		case OPERATING_STATE::DEHUMIDIFYING:
			LOG_DEBUG( "Switching operating state to DEHUMIDIFYING." );
			break;
	}

	this->clear_output( PN_AHU_HEATER );
	this->clear_output( PN_AC_COMPRESSOR );
	this->clear_output( PN_AHU_FAN );

	this->mode_clicks = 0;

	// We use this to delay switching between modes.
	// See: SP MODE SWITCH DELAY
	this->switch_clicks = 0;

	this->op_mode = OPERATING_MODE::NONE;
	this->op_state = _new_state;

	return;
}

/*
	Thread and class lifetime management methods.
*/
void HVAC_LOGIC_LOOP::pre_process( void )
{
	return;
}
void HVAC_LOGIC_LOOP::post_process( void )
{
	return;
}
HVAC_LOGIC_LOOP::HVAC_LOGIC_LOOP( CONFIGURATOR* _config ) : LOGIC_PROCESSOR_BASE( _config )
{
	INIT_LOGGER( "BBB_HVAC::HVAC_LOGIC_LOOP" );
	this->switch_op_state( OPERATING_STATE::NONE );

	this->in_ai_failure = false;
	this->ai_failure_clicks = 0;

	return;
}
HVAC_LOGIC_LOOP::~HVAC_LOGIC_LOOP()
{
	this->op_state = OPERATING_STATE::NONE;
	this->mode_heat = HEAT_MODE::NONE;
	this->mode_cool = COOL_MODE::NONE;
	this->mode_dehum = COOL_MODE::NONE;

	return;
}
