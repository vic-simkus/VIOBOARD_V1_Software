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


	double sp_space_temp =this->get_sp_value("SPACE TEMP");


	//double sp_space_rh = this->get_sp_value("SPACE RH");
	double sp_space_temp_d_high = this->get_sp_value("SPACE TEMP DELTA HIGH");
	double sp_space_temp_d_low = this->get_sp_value("SPACE TEMP DELTA LOW");

	/*
	double sp_space_rh_d = this->get_sp_value("SPACE RH DELTA");
	double sp_space_rh_temp_d = this->get_sp_value("SPACE RH TEMP DELTA");
	*/

	double temp_value = double(int((this->get_ai_value("SPACE_1_TEMP") * 10)))/10;
	double rh_value = double(int((this->get_ai_value("SPACE_1_RH") * 10)))/10;


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
	LOG_DEBUG("++++++++++++++++++++++++++++++++++++++++");

	//LOG_DEBUG("ai_space_1_temp: " + num_to_str(ai_space_1_temp));
	//LOG_DEBUG("ai_space_1_rh: " + num_to_str(ai_space_1_rh));
	//LOG_DEBUG("ai_ahu_supply_temp: " + num_to_str(ai_ahu_supply_temp));
	//LOG_DEBUG("ai_ahu_return_temp: " + num_to_str(ai_ahu_return_temp));


	LOG_DEBUG("Space temp: " + num_to_str(temp_value));

	LOG_DEBUG("Space RH%: " + num_to_str(rh_value));


	LOG_DEBUG("****************************************");

	double delta_calc = temp_value + sp_space_temp_d_high;

	if (delta_calc > sp_space_temp)
	{
		if(this->is_output_set("AC_COMPRESSOR") == true)
		{
			LOG_DEBUG("Space temp + delta: (" + num_to_str(delta_calc) + ") is greater than SP: " + num_to_str(sp_space_temp) + ".  Continuing to cool.");
		}
		else
		{
			LOG_DEBUG("Space temp + delta: (" + num_to_str(delta_calc) + ") is greater than SP: " + num_to_str(sp_space_temp) + ".  Activating space cooling.");

			this->set_output("AC_COMPRESSOR");
			this->set_output("AHU_FAN");
		}

	}
	else
	{
		delta_calc = temp_value + sp_space_temp_d_low;

		if(delta_calc < sp_space_temp)
		{
			LOG_DEBUG("Space temp + delta: (" + num_to_str(delta_calc) + ") is less than SP: " + num_to_str(sp_space_temp) + ".  Activating space heating.");
		}
	}

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
	return;
}

HVAC_LOGIC_LOOP::~HVAC_LOGIC_LOOP()
{
	return;
}
