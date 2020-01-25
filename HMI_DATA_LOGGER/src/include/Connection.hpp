/*
Copyright (C) 2019  Vidas Simkus (vic.simkus@simkus.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __HMI_DATA_LOGGER_CONNECTION_HPP
#define __HMI_DATA_LOGGER_CONNECTION_HPP

#include "lib/bbb_hvac.hpp"

#include <memory>
#include <list>

namespace HMI_DATA_LOGGER
{
	class Context;

	class Connection
	{
		public:

			Connection( HMI_DATA_LOGGER::Context* );
			virtual ~Connection();

			virtual bool connect( void ) = 0;
			virtual bool disconnect( void ) = 0;
			virtual bool read_status( void ) = 0;

			std::list<std::string> get_item_names( void );

		protected:

			bool connect_to_logic_core( void );

			BBB_HVAC::CLIENT::CLIENT_CONTEXT* client_context;
			HMI_DATA_LOGGER::Context* logger_context;
			std::list<std::string> logic_core_points;

		private:
			DEF_LOGGER;
	};
}

#endif