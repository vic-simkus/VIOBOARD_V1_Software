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


#ifndef BOARD_STATE_CACHE_HPP
#define BOARD_STATE_CACHE_HPP

#include <cstdint>
#include <exception>

#include "serial_io_types.hpp"

using namespace std;

namespace BBB_HVAC
{
	using namespace EXCEPTIONS;

	namespace IOCOMM
	{

		class BOARD_STATE_CACHE
		{
		public:

			BOARD_STATE_CACHE();

			void add_pmic_status(uint8_t _value);
			void add_do_status(uint8_t _value);
			void add_adc_value(size_t _x_index, uint16_t _value) throw(logic_error);

			void get_adc_cache(ADC_CACHE_ENTRY(&_dest)[GC_IO_STATE_BUFFER_DEPTH][GC_IO_AI_COUNT]);
			void get_do_cache(DO_CACHE_ENTRY(& _dest)[GC_IO_STATE_BUFFER_DEPTH]);
			void get_pmic_cache(PMIC_CACHE_ENTRY(& _dest)[GC_IO_STATE_BUFFER_DEPTH]);

			void get_latest_adc_values(ADC_CACHE_ENTRY(& _dest)[GC_IO_AI_COUNT]);
			void get_latest_do_status(DO_CACHE_ENTRY& _dest);
			void get_latest_pmic_status(PMIC_CACHE_ENTRY& _dest);

		protected:

			/**
			 * Local cache of the ADC results from the IO board.  Filled automatically on a periodic basis.
			 */
			ADC_CACHE_ENTRY adc_cache[GC_IO_STATE_BUFFER_DEPTH][GC_IO_AI_COUNT];

			/**
			 * Current index into the dac_cache.
			 */
			size_t adc_cache_index;

			/**
			 * Local cache of the digital output states.  We have four digital outputs so we can pack them into a single 8 bit type.
			 */
			DO_CACHE_ENTRY do_cache[GC_IO_STATE_BUFFER_DEPTH];

			PMIC_CACHE_ENTRY pmic_cache[GC_IO_STATE_BUFFER_DEPTH];

			/**
			 * Current index into pmic_cache
			 */
			size_t pmic_cache_index;

			/**
			 * Current index into the do_cache.
			 */
			size_t do_cache_index;

		private:
		};
	}
}
#endif /* BOARD_STATE_CACHE_HPP */

