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

#include "lib/exceptions.hpp"
#include "lib/config.hpp"
#include "lib/globals.hpp"
#include "lib/string_lib.hpp"
#include "lib/configurator.hpp"
#include "lib/threads/logic_thread.hpp"

#include "lib/threads/watchdog_thread.hpp"

#include <signal.h>
#include <string.h>

using namespace BBB_HVAC;

LOGIC_PROCESSOR_BASE::LOGIC_PROCESSOR_BASE(CONFIGURATOR* _config) :
	THREAD_BASE("LOGIC_PROCESSOR_BASE")
{
	this->configurator = _config;
	this->logger = new LOGGING::LOGGER("BBB_HVAC::LOGIC_PROCESSOR_BASE");
}

LOGIC_PROCESSOR_BASE::~LOGIC_PROCESSOR_BASE()
{
	delete this->configurator;
	this->configurator = nullptr;
	delete this->logger;
	this->logger = nullptr;
	return;
}

LOGIC_STATUS_CORE BBB_HVAC::LOGIC_PROCESSOR_BASE::get_logic_status(void)
{
	if(pthread_mutex_trylock(&(this->mutex)) != 0)
	{
		return LOGIC_STATUS_CORE();
	}

	LOGIC_STATUS_CORE ret(this->logic_status_core);
	pthread_mutex_unlock(&(this->mutex));
	return ret;
}

LOGIC_STATUS_FLUFF LOGIC_PROCESSOR_BASE::get_logic_status_fluff(void) const
{
	return this->logic_status_fluff;
}

string LOGIC_STATUS_CORE::to_string(void)
{
	string ret;
	ret += "DI[";

	for(unsigned int i = 0; i < this->di_num; i++)
	{
		ret += num_to_str(this->di_buffer[i]);

		if(i < this->di_num - 1)
		{
			ret += ":";
		}
	}

	ret += "] DO [";

	for(unsigned int i = 0; i < this->do_num; i++)
	{
		ret += num_to_str(this->do_buffer[i]);

		if(i < this->do_num - 1)
		{
			ret += ":";
		}
	}

	ret += "] AI [";

	for(unsigned int i = 0; i < this->ai_num; i++)
	{
		ret += num_to_str(this->ai_buffer[i]);

		if(i < this->ai_num - 1)
		{
			ret += ":";
		}
	}

	ret += "] AO [";

	for(unsigned int i = 0; i < this->ao_num; i++)
	{
		ret += num_to_str(this->ao_buffer[i]);

		if(i < this->ao_num - 1)
		{
			ret += ":";
		}
	}

	ret += "]";
	return ret;
}

bool LOGIC_PROCESSOR_BASE::thread_func(void)
{
	LOG_INFO_P("Starting logic thread.");
	this->obtain_lock();
	this->pre_process();
	this->release_lock();

	while(this->abort_thread == false)
	{
		this->reset_sleep_timespec(GC_LOGIC_THREAD_SLEEP);
		this->nsleep();
		GLOBALS::watchdog->reset_counter();
		this->obtain_lock();

		/*
		 * At this point we have the mutex lock.
		 */

		try
		{
			this->process_logic();
		}
		catch(...)
		{
			LOG_ERROR_P("process_logic() emitted an exception.  Logic thread will abort.");
			this->abort_thread = true;
		}

		/*
		 * Done with the logic processing.  Unlock the mutex so that some other thread may get the status.
		 */
		this->release_lock();
	}

	/*
	 * If we end up here there's two possibilities:
	 * 1 -- The user decided to shut the daemon down
	 * 2 -- Something went to shit.
	 *
	 * Point being, the logic_core is an indeterminate state and the post_process call is merely informational.
	 */
	this->post_process();
	GLOBALS::global_exit_flag = true;
	LOG_INFO_P("Logic thread finished.");
	return true;
}