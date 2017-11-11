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

using namespace BBB_HVAC;

void HVAC_LOGIC_LOOP::process_logic(void) throw(exception)
{
	/*
	 * We do not touch the mutex here.  The thread loop takes care of that
	 * for us before calling process_logic
	 */
	this->logic_status_core.iterations += 1;
	return;
}

void HVAC_LOGIC_LOOP::pre_process(void) throw(exception)
{
	return;
}

void HVAC_LOGIC_LOOP::post_process(void) throw(exception)
{
	return;
}

HVAC_LOGIC_LOOP::HVAC_LOGIC_LOOP(CONFIGURATOR* _config) :
	LOGIC_PROCESSOR_BASE(_config)
{
	this->logic_status_core.ai_num = static_cast<unsigned int>(A_INPUT_PIN_ENUM::__COUNT);
	this->logic_status_core.do_num = static_cast<unsigned int>(D_OUTPUT_PIN_ENUM::__COUNT);
	this->logic_status_core.sp_num = this->configurator->get_sp_index().size();
	/*
	 * How to return labels to the shims in order to implement ???
	 */
	//this->logic_status_fluff.ai_labels
	//this->logic_status_fluff.do_labels
	//this->logic_status_fluff.sp_labels
	{
		const CONFIG_TYPE_INDEX_TYPE& idx = this->configurator->get_ai_index();

		for(CONFIG_TYPE_INDEX_TYPE::const_iterator i = idx.cbegin(); i != idx.cend(); ++i)
		{
			this->logic_status_fluff.ai_labels.push_back(this->configurator->get_config_entry(*i).get_part_as_string(2));
		}
	}
	{
		const CONFIG_TYPE_INDEX_TYPE& idx = this->configurator->get_do_index();

		for(CONFIG_TYPE_INDEX_TYPE::const_iterator i = idx.cbegin(); i != idx.cend(); ++i)
		{
			this->logic_status_fluff.do_labels.push_back(this->configurator->get_config_entry(*i).get_part_as_string(2));
		}
	}
	{
		const CONFIG_TYPE_INDEX_TYPE& idx = this->configurator->get_sp_index();

		for(CONFIG_TYPE_INDEX_TYPE::const_iterator i = idx.cbegin(); i != idx.cend(); ++i)
		{
			this->logic_status_fluff.sp_labels.push_back(this->configurator->get_config_entry(*i).get_part_as_string(0));
		}
	}
	return;
}
HVAC_LOGIC_LOOP::~HVAC_LOGIC_LOOP()
{
	return;
}
