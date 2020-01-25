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

#ifndef __HMI_DATA_LOGGER_CONNECTION_FILE_HPP
#define __HMI_DATA_LOGGER_CONNECTION_FILE_HPP

#include "Connection.hpp"

namespace HMI_DATA_LOGGER
{
	class Context;

	class ConnectionFile : public Connection
	{
		public:
			ConnectionFile( HMI_DATA_LOGGER::Context* );
			virtual ~ConnectionFile();

			virtual bool connect( void );
			virtual bool disconnect( void );
			virtual bool read_status( void );


		protected:

			bool opened_output;

		private:
			DEF_LOGGER;
	};
}
#endif

