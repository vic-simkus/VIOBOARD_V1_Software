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

#include "lib/message_types.hpp"

#include <string>

//PING = 0, PONG, HELLO, READ_STATUS, SET_STATUS, GET_LABELS, __MSG_END__
static std::string __message_type_list[] = { "INVALID", "PING", "PONG", "HELLO", "READ_STATUS", "READ_STATUS_RAW_ANALOG","SET_STATUS", "SET_PMIC_STATUS", "GET_LABELS", "SET_POINT" };

using namespace BBB_HVAC;

__MESSAGE_TYPE::__MESSAGE_TYPE(ENUM_MESSAGE_TYPE _type, const std::string& _label)
{
	this->type = _type;
	this->label = _label;
	return;
}

__MESSAGE_TYPES_INT::__MESSAGE_TYPES_INT()
{
	for(unsigned int i = 0; i != static_cast<unsigned int>(ENUM_MESSAGE_TYPE::__MSG_END__); i++)
	{
		MESSAGE_TYPE mt(new __MESSAGE_TYPE(static_cast<ENUM_MESSAGE_TYPE>(i),__message_type_list[i]));
		this->enum_to_type[static_cast<ENUM_MESSAGE_TYPE>(i)] = mt;
		this->label_to_type[mt->label] = mt;
	}

	return;
}

__MESSAGE_TYPES_INT MESSAGE_TYPE_MAPPER::__internal_mapper;

size_t MESSAGE_TYPE_MAPPER::get_message_type_count(void)
{
	return __internal_mapper.enum_to_type.size();
}

MESSAGE_TYPE MESSAGE_TYPE_MAPPER::get_message_type_by_label(const std::string& _label)
{
	return __internal_mapper.label_to_type[_label];
}
MESSAGE_TYPE MESSAGE_TYPE_MAPPER::get_message_type_by_enum(const ENUM_MESSAGE_TYPE& _enum)
{
	return __internal_mapper.enum_to_type[_enum];
}

void MESSAGE_TYPE_MAPPER::dump_supported_messages(std::ostream& out)
{
	out << "Supported message types:" << std::endl;
	out << "--- start of listing --" << std::endl;

	for(unsigned int i = 0; i < MESSAGE_TYPE_MAPPER::get_message_type_count(); i++)
	{
		out.width(2);
		out.fill('0');
		out << i << ": " << MESSAGE_TYPE_MAPPER::get_message_type_by_enum(static_cast<ENUM_MESSAGE_TYPE>(i))->to_string() << std::endl;
	}

	out << "--- end of listing --" << std::endl;
	out << std::endl;
}
