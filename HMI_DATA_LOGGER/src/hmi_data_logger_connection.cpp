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

#include "include/hmi_data_logger_connection.hpp"

using namespace HMI_DATA_LOGGER;

HMI_DATA_LOGGER_CONNECTION::HMI_DATA_LOGGER_CONNECTION()
{
	INIT_LOGGER("HMI_DATA_LOGGER::HMIC_DATA_LOGGER_CONNECTION");

	LOG_DEBUG("Instantiating.");

	this->ctx.reset(BBB_HVAC::CLIENT::CLIENT_CONTEXT::create_instance());

}

bool HMI_DATA_LOGGER_CONNECTION::connect(void)
{
	try
	{
		this->ctx->connect();
	}
	catch(const BBB_HVAC::EXCEPTIONS::CONNECTION_ERROR& e)
	{
		LOG_ERROR("Failed to connect to logic core: " + std::string(e.what()));
		return false;
	}

	return true;
}

HMI_DATA_LOGGER_CONNECTION::~HMI_DATA_LOGGER_CONNECTION()
{
	LOG_DEBUG("Destroying.");

	if(this->ctx)
	{
		this->ctx->disconnect();
	}


}

