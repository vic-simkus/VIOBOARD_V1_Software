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

#ifndef OUTGOING_MESSAGE_QUEUE_HPP
#define OUTGOING_MESSAGE_QUEUE_HPP

#include <queue>
#include <string>
#include <cstdint>
#include <pthread.h>

#include "lib/tprotect_base.hpp"
#include "lib/exceptions.hpp"
#include "logger.hpp"

namespace BBB_HVAC
{
	using namespace EXCEPTIONS;

	namespace IOCOMM
	{
		/**
		 * A queue of outgoing messages waiting to be written to the wire.
		 */
		class OUTGOING_MESSAGE_QUEUE : protected TPROTECT_BASE
		{
		public:
			OUTGOING_MESSAGE_QUEUE(const std::string& _tag);
			virtual ~OUTGOING_MESSAGE_QUEUE();

			bool add_message(const OUTGOING_MESSAGE& _msg);

			bool wait_for_signal(void) throw(LOCK_ERROR);

			bool has_more_messages(void) const;
			OUTGOING_MESSAGE get_message(void);
			void put_lock(void) throw(LOCK_ERROR);

			inline void swap_message_queue(std::queue<OUTGOING_MESSAGE>* _destination) {
				std::swap(this->message_queue, *_destination);
				return;
			}

			inline void clear(void)
			{
				while(!this->message_queue.empty())
				{
					this->message_queue.pop();
				}
			}
		protected:

			void signal(void);
			void get_lock(void) throw(LOCK_ERROR);



			pthread_cond_t conditional;

			std::string tag;
			std::queue<OUTGOING_MESSAGE> message_queue;
			uint_least32_t id_seq;
		private:

			LOGGING::LOGGER* logger;
		};

	}
}
#endif /* OUTGOING_MESSAGE_QUEUE_HPP */

