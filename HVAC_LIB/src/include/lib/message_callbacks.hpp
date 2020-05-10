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


#ifndef SRC_INCLUDE_LIB_MESSAGE_CALLBACKS_HPP_
#define SRC_INCLUDE_LIB_MESSAGE_CALLBACKS_HPP_

#include "lib/logger.hpp"
#include "lib/message_lib.hpp"
#include "lib/exceptions.hpp"
#include "lib/hvac_types.hpp"

namespace BBB_HVAC
{
	class BASE_CONTEXT;

	/**
	 * Interface for implementing a message callback.
	 * When a new message comes in the comm thread will call call process_message to notify the consuming logic of the new message.
	 */
	class MESSAGE_CALLBACK_BASE
	{
		public:
			virtual ~MESSAGE_CALLBACK_BASE();
			virtual ENUM_MESSAGE_CALLBACK_RESULT process_message( ENUM_MESSAGE_DIRECTION _direction, BASE_CONTEXT* _ctx, const MESSAGE_PTR& _message ) = 0;

		protected:
		private:
	};
}

#endif /* SRC_INCLUDE_LIB_MESSAGE_CALLBACKS_HPP_ */
