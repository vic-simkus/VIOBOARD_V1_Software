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
#include <vector>

namespace HMI_DATA_LOGGER
{
	class HMI_DATA_LOGGER_CONTEXT;

	class HMI_DATA_LOGGER_CONNECTION
	{
		public:

			HMI_DATA_LOGGER_CONNECTION( HMI_DATA_LOGGER::HMI_DATA_LOGGER_CONTEXT* );
			~HMI_DATA_LOGGER_CONNECTION();

			virtual bool connect( void ) = 0;
			virtual bool disconnect( void ) = 0;
			virtual bool read_status( void ) = 0;

		protected:
		private:
			DEF_LOGGER;
			BBB_HVAC::CLIENT::CLIENT_CONTEXT* client_context;
			HMI_DATA_LOGGER::HMI_DATA_LOGGER_CONTEXT* logger_context;

			std::vector<std::string> logic_core_points;

			bool opened_output;
	};
}

#endif