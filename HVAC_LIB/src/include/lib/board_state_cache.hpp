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
#include <functional>

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
			typedef std::function<void(BOARD_STATE_CACHE&, size_t,uint16_t)> CAL_VALUE_ADDER_PTR;

			BOARD_STATE_CACHE();

			void add_pmic_status(uint8_t _value);
			void add_do_status(uint8_t _value);
			void add_adc_value(size_t _x_index, uint16_t _value) throw(logic_error);

			void add_l1_cal_value(size_t _x_index, uint16_t _value) throw(logic_error);
			void add_l2_cal_value(size_t _x_index, uint16_t _value) throw(logic_error);

			void set_boot_count(uint16_t _value);

			void get_adc_cache(ADC_CACHE_ENTRY(&_dest)[GC_IO_STATE_BUFFER_DEPTH][GC_IO_AI_COUNT]);
			void get_do_cache(DO_CACHE_ENTRY(& _dest)[GC_IO_STATE_BUFFER_DEPTH]);
			void get_pmic_cache(PMIC_CACHE_ENTRY(& _dest)[GC_IO_STATE_BUFFER_DEPTH]);

			uint16_t get_boot_count(void) const;

			void get_latest_adc_values(ADC_CACHE_ENTRY(& _dest)[GC_IO_AI_COUNT]);
			void get_latest_do_status(DO_CACHE_ENTRY& _dest);
			void get_latest_pmic_status(PMIC_CACHE_ENTRY& _dest);

			void get_latest_l1_cal_values(CAL_VALUE_ENTRY(& _dest)[GC_IO_AI_COUNT]) const;
			void get_latest_l2_cal_values(CAL_VALUE_ENTRY(& _dest)[GC_IO_AI_COUNT]) const;

		protected:

			/**
			 * Local cache of the ADC results from the IO board.  Filled automatically on a periodic basis.
			 */
			ADC_CACHE_ENTRY adc_cache[GC_IO_STATE_BUFFER_DEPTH][GC_IO_AI_COUNT];

			/**
			 * Local cache of the digital output states.  We have four digital outputs so we can pack them into a single 8 bit type.
			 */
			DO_CACHE_ENTRY do_cache[GC_IO_STATE_BUFFER_DEPTH];

			PMIC_CACHE_ENTRY pmic_cache[GC_IO_STATE_BUFFER_DEPTH];

			CAL_VALUE_ENTRY cal_l1_cache[GC_IO_STATE_BUFFER_DEPTH][GC_IO_AI_COUNT];

			CAL_VALUE_ENTRY cal_l2_cache[GC_IO_STATE_BUFFER_DEPTH][GC_IO_AI_COUNT];

			uint16_t boot_count;

			/**
			 * Current index into pmic_cache
			 */
			size_t pmic_cache_index;

			/**
			 * Current index into the do_cache.
			 */
			size_t do_cache_index;

			size_t l1_cal_cache_index;

			size_t l2_cal_cache_index;

			/**
			 * Current index into the dac_cache.
			 */
			size_t adc_cache_index;

		private:
			void add_cal_value(size_t _x_index, uint16_t _value,size_t& _idx,CAL_VALUE_ENTRY(& _dest)[GC_IO_STATE_BUFFER_DEPTH][GC_IO_AI_COUNT]) throw(logic_error);
		};
	}
}
#endif /* BOARD_STATE_CACHE_HPP */

