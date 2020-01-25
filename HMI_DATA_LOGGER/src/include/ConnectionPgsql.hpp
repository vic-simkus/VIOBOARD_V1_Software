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

#ifndef __HMI_DATA_LOGGER_CONNECTION_PGSQL_HPP
#define __HMI_DATA_LOGGER_CONNECTION_PGSQL_HPP

#include "Connection.hpp"

namespace  HMI_DATA_LOGGER
{
	class Context;

	class ConnectionPgsql : public Connection
	{
		public:
			ConnectionPgsql( HMI_DATA_LOGGER::Context* );
			~ConnectionPgsql();
			virtual bool connect( void );
			virtual bool disconnect( void );
			virtual bool read_status( void );
		protected:
		private:
	};
}
#endif
