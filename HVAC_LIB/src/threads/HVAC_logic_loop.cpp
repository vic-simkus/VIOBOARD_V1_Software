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

void HVAC_LOGIC_LOOP::process_logic( void ) throw( exception )
{
	/*
	 * We do not touch the mutex here.  The thread loop takes care of that
	 * for us before calling process_logic
	 */
	this->logic_status_core.iterations += 1;
	/*
	Space temperature set point
	*/
	double sp_space_temp = this->get_sp_value( "SPACE TEMP" );
	double sp_space_rh = this->get_sp_value( "SPACE RH" );
	double sp_space_temp_d_high = this->get_sp_value( "SPACE TEMP DELTA HIGH" );
	double sp_space_temp_d_low = this->get_sp_value( "SPACE TEMP DELTA LOW" );
	double sp_space_rh_d = this->get_sp_value( "SPACE RH DELTA" );
	double sp_space_rh_temp_d = this->get_sp_value( "SPACE RH TEMP DELTA" );
	unsigned int sp_ahu_delay_cooling_clicks = ( unsigned int )this->get_sp_value( "AHU FAN DELAY COOLING" );
	unsigned int sp_ahu_delay_heating_clicks = ( unsigned int )this->get_sp_value( "AHU FAN DELAY HEATING" );
	/*
	Get the temperature and relative humidity value and round it off to one decimal place.
	*/
	double temp_value = double( int( ( this->get_ai_value( "SPACE_1_TEMP" ) * 10 ) ) ) / 10;
	double rh_value = double( int( ( this->get_ai_value( "SPACE_1_RH" ) * 10 ) ) ) / 10;
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
	//LOG_DEBUG( "Space RH%: " + num_to_str( rh_value ) );
	//LOG_DEBUG( "****************************************" );
	//LOG_DEBUG( "Space temp: " + num_to_str( temp_value ) + "F, space RH: " + num_to_str( rh_value ) + "%" );
	double heating_point = sp_space_temp + sp_space_temp_d_low;
	double cooling_point = sp_space_temp + sp_space_temp_d_high;
	double dehum_action_point = sp_space_rh + sp_space_rh_d;
	double dehum_cancel_temp_point = sp_space_temp - sp_space_rh_temp_d;

	if ( this->op_state == OPERATING_STATE::NONE )
	{
		LOG_DEBUG( "op_state: NONE" );
		this->clear_output( "AHU_HEATER" );
		this->clear_output( "AC_COMPRESSOR" );
		this->clear_output( "AHU_FAN" );
		this->ahu_delay_clicks = 0;

		if ( temp_value >= cooling_point )
		{
			LOG_DEBUG( "Space temp " + num_to_str( temp_value ) + " is greater than cooling switch over point: " + num_to_str( cooling_point ) + ".  Switching to cooling mode." );
			this->op_state = OPERATING_STATE::COOLING;
			goto _end_func;
		}
		else if ( temp_value <= heating_point )
		{
			LOG_DEBUG( "Space temp " + num_to_str( temp_value ) + " is less than heating switch over point: " + num_to_str( heating_point ) + ".  Switching to heating mode." );
			this->op_state = OPERATING_STATE::HEATING;
			goto _end_func;
		}
		else if ( rh_value > dehum_action_point )
		{
			LOG_DEBUG( "Space RH " + num_to_str( rh_value ) + " is greater than dehumidification action point: " + num_to_str( dehum_action_point ) + ".  Switching to dehumidification mode." );
			this->op_state = OPERATING_STATE::DEHUMIDIFYING;
			goto _end_func;
		}
		else
		{
			LOG_DEBUG( "Leaving current operational state as NONE" );
			goto _end_func;
		}
	}
	else if ( this->op_state == OPERATING_STATE::HEATING )
	{
		LOG_DEBUG( "op_state: HEATING" );

		if ( temp_value >= heating_point )
		{
			this->op_state = OPERATING_STATE::NONE;
		}
		else
		{
			if ( this->ahu_delay_clicks >= sp_ahu_delay_heating_clicks )
			{
				this->set_output( "AHU_FAN" );
			}
			else
			{
				this->set_output( "AHU_HEATER" );
				this->ahu_delay_clicks += 1;
			}
		}
	}
	else if ( this->op_state == OPERATING_STATE::COOLING )
	{
		LOG_DEBUG( "op_state: COOLING" );

		if ( temp_value <= cooling_point )
		{
			this->op_state = OPERATING_STATE::NONE;
		}
		else
		{
			if ( this->ahu_delay_clicks >= sp_ahu_delay_cooling_clicks )
			{
				this->set_output( "AHU_FAN" );
			}
			else
			{
				this->set_output( "AC_COMPRESSOR" );
				this->ahu_delay_clicks += 1;
			}
		}
	}
	else if ( this->op_state == OPERATING_STATE::DEHUMIDIFYING )
	{
		LOG_DEBUG( "op_state: DEHUMIDIFYING" );

		if ( ( rh_value <= sp_space_rh ) || ( temp_value <= dehum_cancel_temp_point ) )
		{
			this->op_state = OPERATING_STATE::NONE;
		}
		else
		{
			if ( this->ahu_delay_clicks >= sp_ahu_delay_cooling_clicks )
			{
				this->set_output( "AHU_FAN" );
			}
			else
			{
				this->set_output( "AC_COMPRESSOR" );
				this->ahu_delay_clicks += 1;
			}
		}
	}
	else
	{
		LOG_ERROR( "Unrecognized operational mode: " + num_to_str( ( unsigned char )this->op_state ) );
	}

_end_func:
	return;
}

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
	this->op_state = OPERATING_STATE::NONE;
	this->ahu_delay_clicks = 0;
	return;
}
HVAC_LOGIC_LOOP::~HVAC_LOGIC_LOOP()
{
	this->op_state = OPERATING_STATE::NONE;
	this->ahu_delay_clicks = 0;
	return;
}