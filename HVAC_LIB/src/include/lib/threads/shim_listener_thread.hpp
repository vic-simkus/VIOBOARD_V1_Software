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

#ifndef SHIM_LISTENER_THREAD_HPP
#define SHIM_LISTENER_THREAD_HPP

#include "lib/logger.hpp"
#include "lib/threads/thread_base.hpp"
#include "lib/exceptions.hpp"

namespace BBB_HVAC
{
	namespace SERVER
	{
		class HS_SERVER_CONTEXT;
	}
	class SHIM_LISTENER : public THREAD_BASE
	{
	public:
		SHIM_LISTENER();
		~SHIM_LISTENER();

		void init( void ) throw( exception );
	protected:
		bool thread_func( void );

	private:
		SERVER::HS_SERVER_CONTEXT* server_ctx;
		LOGGING::LOGGER* logger;

	};
}

#endif /* SHIM_LISTENER_THREAD_HPP */

