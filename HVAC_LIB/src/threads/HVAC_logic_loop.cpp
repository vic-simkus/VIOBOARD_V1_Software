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
using namespace BBB_HVAC;
using namespace BBB_HVAC::HVAC_LOGIC;


HVAC_LOGIC_LOOP::HVAC_LOOP_INVOCATION_CONTEXT::HVAC_LOOP_INVOCATION_CONTEXT( HVAC_LOGIC_LOOP* _parent )
{
	/*
	Space temperature set point
	*/
	this->sp_space_temp = ( float )_parent->get_sp_value( SP_SPACE_TEMP );
	this->sp_space_rh = ( float )_parent->get_sp_value( SP_SPACE_RH );
	this->sp_space_temp_d_high = ( float )_parent->get_sp_value( SP_SPACE_TEMP_DELTA_HIGH );
	this->sp_space_temp_d_low = ( float )_parent->get_sp_value( SP_SPACE_TEMP_DELTA_LOW );
	this->sp_space_rh_d = ( float )_parent->get_sp_value( SP_SPACE_RH_DELTA );
	this->sp_space_rh_temp_d = ( float )_parent->get_sp_value( SP_SPACE_RH_TEMP_DELTA );
	this->sp_ahu_delay_cooling = ( unsigned int )_parent->get_sp_value( SP_AHU_FAN_DELAY_COOLING );
	this->sp_ahu_delay_heating = ( unsigned int )_parent->get_sp_value( SP_AHU_FAN_DELAY_HEATING );
	this->sp_mode_switch_delay = ( unsigned int )_parent->get_sp_value( SP_MODE_SWITCH_DELAY );
	this->sp_heating_deadband = ( unsigned int )_parent->get_sp_value( SP_HEATING_DEADBAND );
	this->sp_cooling_deadband = ( unsigned int )_parent->get_sp_value( SP_COOLING_DEADBAND );

	this->sp_cooling_setpoint_delay = ( unsigned int )_parent->get_sp_value( SP_COOLING_SETPOINT_DELAY );
	this->sp_heating_setpoint_delay = ( unsigned int )_parent->get_sp_value( SP_HEATING_SETPOINT_DELAY );;

	/*
	Get the temperature and relative humidity value and round it off to one decimal place.
	*/
	this->temp_value = float( int( ( _parent->get_ai_value( AI_SPACE_1_TEMP ) * 10 ) ) ) / 10;
	this->rh_value = float( int( ( _parent->get_ai_value( AI_SPACE_1_RH ) * 10 ) ) ) / 10;

	//double ai_ahu_supply_temp = this->get_ai_value("AHU_SUPPLY_TEMP");
	//double ai_ahu_return_temp = this->get_ai_value("AHU_RETURN_TEMP");
	/*
	LOG_DEBUG("sp_space_temp: " + num_to_str(sp_space_temp));
	LOG_DEBUG("sp_space_rh: " + num_to_str(sp_space_rh));
	LOG_DEBUG("sp_space_temp_d_high: " + num_to_str(sp_space_temp_d_high));
	LOG_DEBUG("sp_space_temp_d_low: " + num_to_str(sp_space_temp_d_low));
	LOG_DEBUG("sp_space_rh_d: " + num_to_str(sp_space_rh_d));
	LOG_DEBUG("sp_space_rh_temp_d: " + num_to_str(sp_space_rh_temp_d));
	*/
	//LOG_DEBUG( "++++++++++++++++++++++++++++++++++++++++" );
	//LOG_DEBUG("ai_space_1_temp: " + num_to_str(ai_space_1_temp));
	//LOG_DEBUG("ai_space_1_rh: " + num_to_str(ai_space_1_rh));
	//LOG_DEBUG("ai_ahu_supply_temp: " + num_to_str(ai_ahu_supply_temp));
	//LOG_DEBUG("ai_ahu_return_temp: " + num_to_str(ai_ahu_return_temp));
	//LOG_DEBUG( "Space temp: " + num_to_str( temp_value ) );
	//LOG_DEBUG( "Space RH % : " + num_to_str( rh_value ) );
	//LOG_DEBUG( "**************************************** " );
	//LOG_DEBUG( "Space temp: " + num_to_str( temp_value ) + "F, space RH: " + num_to_str( rh_value ) + " % " );

	// The space temp delta low is specified as a negative integer.
	this->heating_point = sp_space_temp + sp_space_temp_d_low;
	this->cooling_point = sp_space_temp + sp_space_temp_d_high;
	this->dehum_action_point = sp_space_rh + sp_space_rh_d;
	this->dehum_cancel_temp_point = sp_space_temp - sp_space_rh_temp_d;

	return;
}
void HVAC_LOGIC_LOOP::process_logic( void ) throw( exception )
{
	/*
	 * We do not touch the mutex here.  The thread loop takes care of that
	 * for us before calling process_logic
	 */
	this->logic_status_core.iterations += 1;

	const HVAC_LOOP_INVOCATION_CONTEXT ictx( this );

	switch ( this->op_state )
	{
		case OPERATING_STATE::NONE:
			this->process_logic_none( ictx );
			break;

		case OPERATING_STATE::HEATING:
			this->process_logic_heating( ictx );
			break;

		case OPERATING_STATE::COOLING:
			this->process_logic_cooling( ictx );
			break;

		case OPERATING_STATE::DEHUMIDIFYING:
			this->process_logic_dehumidification( ictx );
			break;
	};

	return;
}

void HVAC_LOGIC_LOOP::switch_op_state( OPERATING_STATE _new_state )
{
	switch ( _new_state )
	{
		case OPERATING_STATE::NONE:
			LOG_DEBUG( "Switching operating state to NONE." );

			this->clear_output( PN_AHU_HEATER );
			this->clear_output( PN_AC_COMPRESSOR );
			this->clear_output( PN_AHU_FAN );
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

	this->mode_heat = HEAT_MODE::NONE;
	this->mode_cool = COOL_MODE::NONE;

	this->set_outputs_for_heating( this->mode_heat );
	this->set_outputs_for_cooling( this->mode_cool );

	this->cool_mode_clicks = 0;
	this->heat_mode_clicks = 0;

	this->switch_clicks = 0;

	this->op_state = _new_state;

	return;
}

void HVAC_LOGIC_LOOP::set_outputs_for_heating( const HEAT_MODE _mode )
{
	switch ( _mode )
	{
		case HEAT_MODE::NONE:
			this->clear_output( PN_AC_COMPRESSOR );
			this->clear_output( PN_AHU_HEATER );
			this->clear_output( PN_AHU_FAN );
			break;

		case HEAT_MODE::DELAY_ON:
			this->clear_output( PN_AC_COMPRESSOR );
			this->set_output( PN_AHU_HEATER );
			this->clear_output( PN_AHU_FAN );
			break;

		case HEAT_MODE::HEATING:
			this->clear_output( PN_AC_COMPRESSOR );
			this->set_output( PN_AHU_HEATER );
			this->set_output( PN_AHU_FAN );
			break;

		case HEAT_MODE::DELAY_OFF:
			this->clear_output( PN_AC_COMPRESSOR );
			this->clear_output( PN_AHU_HEATER );
			this->set_output( PN_AHU_FAN );
			break;
	};

	return;
}
void HVAC_LOGIC_LOOP::set_outputs_for_cooling( const COOL_MODE _mode )
{
	switch ( _mode )
	{
		case COOL_MODE::NONE:
			this->clear_output( PN_AC_COMPRESSOR );
			this->clear_output( PN_AHU_HEATER );
			this->clear_output( PN_AHU_FAN );
			break;

		case COOL_MODE::DELAY_ON:
			this->set_output( PN_AC_COMPRESSOR );
			this->clear_output( PN_AHU_FAN );
			this->clear_output( PN_AHU_HEATER );
			break;

		case COOL_MODE::COOLING:
			this->set_output( PN_AC_COMPRESSOR );
			this->set_output( PN_AHU_FAN );
			this->clear_output( PN_AHU_HEATER );
			break;

		case COOL_MODE::DELAY_OFF:
			this->clear_output( PN_AC_COMPRESSOR );
			this->set_output( PN_AHU_FAN );
			this->clear_output( PN_AHU_HEATER );
			break;
	};

	return;
}

void HVAC_LOGIC_LOOP::switch_heating_mode( HEAT_MODE _new_mode )
{
	this->set_outputs_for_heating( _new_mode );

	this->mode_heat = _new_mode;
	this->heat_mode_clicks = 0;
	return;
}
void HVAC_LOGIC_LOOP::switch_cooling_mode( COOL_MODE _new_mode )
{
	this->mode_cool = _new_mode;
	this->cool_mode_clicks = 0;
	return;
}

void HVAC_LOGIC_LOOP::process_logic_none( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx )
{
	if ( this->switch_clicks >= _ctx.sp_mode_switch_delay )
	{
		if ( _ctx.temp_value >= _ctx.cooling_point )
		{
			LOG_DEBUG( "Space temp " + num_to_str( _ctx.temp_value ) + " is greater than cooling switch over point: " + num_to_str( _ctx.cooling_point ) + ".  Switching to cooling mode." );
			this->switch_op_state( OPERATING_STATE::COOLING );
		}
		else if ( _ctx.temp_value <= _ctx.heating_point )
		{
			LOG_DEBUG( "Space temp " + num_to_str( _ctx.temp_value ) + " is less than heating switch over point: " + num_to_str( _ctx.heating_point ) + ".  Switching to heating mode." );
			this->switch_op_state( OPERATING_STATE::HEATING );
		}
		else if ( _ctx.rh_value > _ctx.dehum_action_point )
		{
			LOG_DEBUG( "Space RH " + num_to_str( _ctx.rh_value ) + " is greater than dehumidification action point: " + num_to_str( _ctx.dehum_action_point ) + ".  Switching to dehumidification mode." );
			this->switch_op_state( OPERATING_STATE::DEHUMIDIFYING );
		}
	}
	else
	{
		this->switch_clicks += 1;
	}

	return;
}
void HVAC_LOGIC_LOOP::process_logic_cooling( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx )
{
	this->set_outputs_for_cooling( this->mode_cool );

	switch ( this->mode_cool )
	{
		case COOL_MODE::NONE:
		{
			// Initial entry into the heating logic.
			this->switch_cooling_mode( COOL_MODE::DELAY_ON );

			break;
		}

		case COOL_MODE::DELAY_ON:
		{
			this->cool_mode_clicks += 1;

			if ( this->cool_mode_clicks >= _ctx.sp_ahu_delay_cooling )
			{
				this->switch_cooling_mode( COOL_MODE::COOLING );
			}

			break;
		}

		case COOL_MODE::COOLING:
		{
			if ( _ctx.temp_value <= ( ( ( float )_ctx.cooling_point - ( float )_ctx.sp_cooling_deadband ) ) )
			{
				LOG_DEBUG( "temp_value is " + num_to_str( _ctx.temp_value ) + " which is <=  than " + num_to_str( ( ( float ) _ctx.cooling_point - ( float )_ctx.sp_cooling_deadband ) ) + ".  click: " + num_to_str( this->cool_mode_clicks ) ) ;
				this->cool_mode_clicks += 1;

				// Current space temp is above heating point plus deadband
				if ( this->cool_mode_clicks >= _ctx.sp_cooling_setpoint_delay )
				{
					this->switch_cooling_mode( COOL_MODE::DELAY_OFF );
				}
			}
			else
			{
				this->cool_mode_clicks = 0;
			}

			break;
		}

		case COOL_MODE::DELAY_OFF:
		{
			this->cool_mode_clicks += 1;

			if ( this->cool_mode_clicks >= _ctx.sp_ahu_delay_cooling )
			{
				this->switch_cooling_mode( COOL_MODE::NONE );
				this->switch_op_state( OPERATING_STATE::NONE );
			}

			break;
		}
	};

	return;
}
void HVAC_LOGIC_LOOP::process_logic_heating( const HVAC_LOOP_INVOCATION_CONTEXT& _ctx )
{
	this->set_outputs_for_heating( this->mode_heat );

	switch ( this->mode_heat )
	{
		case HEAT_MODE::NONE:
		{
			// Initial entry into the heating logic.
			this->switch_heating_mode( HEAT_MODE::DELAY_ON );

			break;
		}

		case HEAT_MODE::DELAY_ON:
		{
			this->heat_mode_clicks += 1;

			if ( this->heat_mode_clicks >= _ctx.sp_ahu_delay_heating )
			{
				this->switch_heating_mode( HEAT_MODE::HEATING );
			}

			break;
		}

		case HEAT_MODE::HEATING:
		{
			if ( _ctx.temp_value >= ( ( float )_ctx.heating_point + ( float )_ctx.sp_heating_deadband ) )
			{
				LOG_DEBUG( "temp_value is " + num_to_str( _ctx.temp_value ) + " which is <=  than " + num_to_str( ( ( float ) _ctx.heating_point + ( float )_ctx.sp_heating_deadband ) ) + ".  click: " + num_to_str( this->heat_mode_clicks ) ) ;
				this->heat_mode_clicks += 1;

				// Current space temp is above heating point plus deadband
				if ( this->heat_mode_clicks >= _ctx.sp_heating_setpoint_delay )
				{
					this->switch_heating_mode( HEAT_MODE::DELAY_OFF );
				}
			}
			else
			{
				this->heat_mode_clicks = 0;
			}

			break;
		}

		case HEAT_MODE::DELAY_OFF:
		{
			this->heat_mode_clicks += 1;

			if ( this->heat_mode_clicks >= _ctx.sp_ahu_delay_heating )
			{
				this->switch_heating_mode( HEAT_MODE::NONE );
				this->switch_op_state( OPERATING_STATE::NONE );
			}

			break;
		}
	}

	return;
}
void HVAC_LOGIC_LOOP::process_logic_dehumidification( const HVAC_LOOP_INVOCATION_CONTEXT& )
{

}

/*
	Thread and class lifetime management methods.
*/
void HVAC_LOGIC_LOOP::pre_process( void ) throw( exception )
{
	return;
}
void HVAC_LOGIC_LOOP::post_process( void ) throw( exception )
{
	return;
}
HVAC_LOGIC_LOOP::HVAC_LOGIC_LOOP( CONFIGURATOR* _config ) : LOGIC_PROCESSOR_BASE( _config )
{
	this->switch_op_state( OPERATING_STATE::NONE );

	return;
}
HVAC_LOGIC_LOOP::~HVAC_LOGIC_LOOP()
{
	this->op_state = OPERATING_STATE::NONE;
	this->mode_heat = HEAT_MODE::NONE;
	this->mode_cool = COOL_MODE::NONE;

	return;
}