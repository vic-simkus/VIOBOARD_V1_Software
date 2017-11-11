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

#include "lib/config.hpp"

#include "lib/logger.hpp"
#include "lib/threads/watchdog_thread.hpp"

#include "lib/threads/thread_registry.hpp"
#include "lib/globals.hpp"
#include <unistd.h>
#include <signal.h>

using namespace BBB_HVAC;

namespace BBB_HVAC
{

	WATCHDOG::WATCHDOG() : THREAD_BASE("WATCHDOG")
	{
		this->logger = new LOGGING::LOGGER();
		INIT_LOGGER_P("BBB_HVAC::WATCHDOG_THREAD");
		this->counter = 0;
		return;
	}

	void WATCHDOG::reset_counter(void)
	{
		this->counter = 0;
	}

	WATCHDOG::~WATCHDOG()
	{
		delete this->logger;
		this->logger = nullptr;
		return;
	}

	bool WATCHDOG::thread_func(void)
	{
		this->reset_sleep_timespec(GC_WATCHDOG_SLEEP_NSEC);

		while(this->abort_thread == false)
		{
			this->nsleep();
			THREAD_REGISTRY::init_cleanup();

			if(this->counter >= GC_WATCHDOG_ATTEMPTS)
			{
				LOG_ERROR_P("Watchdog thread terminating program.");
				kill(0,SIGTERM);
				return false;
			}

			this->counter += 1;
		}

		return true;
	}
}
