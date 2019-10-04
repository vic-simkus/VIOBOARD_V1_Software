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

#include "lib/board_state_cache.hpp"

namespace BBB_HVAC
{
	namespace IOCOMM
	{
		BOARD_STATE_CACHE::BOARD_STATE_CACHE()
		{
			this->pmic_cache_index = 0;
			this->adc_cache_index = 0;
			this->do_cache_index = 0;
			this->l1_cal_cache_index = 0;
			this->l2_cal_cache_index = 0;

			for ( unsigned int i = 0; i < GC_IO_AI_COUNT; i++ )
			{
				this->forced_ai_value[i] = false;
			}

			return;
		}

		void BOARD_STATE_CACHE::add_pmic_status( uint8_t _value )
		{
			this->pmic_cache[this->pmic_cache_index] = PMIC_CACHE_ENTRY( _value );
			this->pmic_cache_index += 1;

			if ( this->pmic_cache_index == GC_IO_STATE_BUFFER_DEPTH )
			{
				this->pmic_cache_index = 0;
			}

			return;
		}

		void BOARD_STATE_CACHE::add_do_status( uint8_t _value )
		{
			this->do_cache[this->do_cache_index] = DO_CACHE_ENTRY( _value );
			this->do_cache_index += 1;

			if ( this->do_cache_index == GC_IO_STATE_BUFFER_DEPTH )
			{
				this->do_cache_index = 0;
			}

			return;
		}

		size_t BOARD_STATE_CACHE::get_previous_cache_index( void )
		{
			if ( this->adc_cache_index == 0 )
			{
				return ( GC_IO_STATE_BUFFER_DEPTH - 1 );
			}
			else
			{
				return ( this->adc_cache_index - 1 );
			}
		}

		bool BOARD_STATE_CACHE::force_ai_value( size_t _x_index, uint16_t _value )
		{
			if ( this->forced_ai_value[_x_index] == true )
			{
				return false;
			}

			this->adc_cache[this->get_previous_cache_index()][_x_index] = ADC_CACHE_ENTRY( _value );
			return true;
		}
		bool BOARD_STATE_CACHE::unforce_ai_value( size_t _x_index )
		{
			if ( this->forced_ai_value[_x_index] == false )
			{
				return false;
			}

			this->forced_ai_value[_x_index] = false;
			return true;
		}

		void BOARD_STATE_CACHE::add_adc_value( size_t _x_index, uint16_t _value ) throw( logic_error )
		{
			if ( _x_index >= GC_IO_AI_COUNT )
			{
				throw out_of_range( "Supplied x_index " + num_to_str( _x_index ) + " is greater than number of defined analog inputs.  See GC_IO_AI_COUNT." );
			}

			if ( this->forced_ai_value[_x_index] == false )
			{
				/*
				Value is not forced.  Set value
				*/
				this->adc_cache[this->adc_cache_index][_x_index] = ADC_CACHE_ENTRY( _value );
			}
			else
			{
				/*
				Value is forced.  Do not set value.  Updated timestamp.
				*/
				this->adc_cache[this->adc_cache_index][_x_index] = ADC_CACHE_ENTRY( this->adc_cache[this->get_previous_cache_index()][_x_index] );
			}

			if ( _x_index == GC_IO_AI_COUNT - 1 )
			{
				this->adc_cache_index += 1;
			}

			if ( this->adc_cache_index == GC_IO_STATE_BUFFER_DEPTH )
			{
				this->adc_cache_index = 0;
			}

			return;
		}

		void BOARD_STATE_CACHE::add_l1_cal_value( size_t _x_index, uint16_t _value ) throw( logic_error )
		{
			this->add_cal_value( _x_index, _value, this->l1_cal_cache_index, ( this->cal_l1_cache ) );
		}
		void BOARD_STATE_CACHE::add_l2_cal_value( size_t _x_index, uint16_t _value ) throw( logic_error )
		{
			this->add_cal_value( _x_index, _value, this->l1_cal_cache_index, ( this->cal_l2_cache ) );
		}

		void BOARD_STATE_CACHE::add_cal_value( size_t _x_index, uint16_t _value, size_t& _idx, CAL_VALUE_ENTRY( & _dest ) [GC_IO_STATE_BUFFER_DEPTH][GC_IO_AI_COUNT] ) throw( logic_error )
		{
			if ( _x_index >= GC_IO_AI_COUNT )
			{
				throw out_of_range( "Supplied x_index is greater than number of defined analog inputs.  See GC_IO_AI_COUNT." );
			}

			_dest[_idx][_x_index] = CAL_VALUE_ENTRY( _value );

			if ( _x_index == GC_IO_AI_COUNT - 1 )
			{
				_idx += 1;
			}

			if ( _idx == GC_IO_STATE_BUFFER_DEPTH )
			{
				_idx = 0;
			}

			return;
		}

		void BOARD_STATE_CACHE::get_adc_cache( ADC_CACHE_ENTRY( &_dest ) [GC_IO_STATE_BUFFER_DEPTH][GC_IO_AI_COUNT] )
		{
			for ( int i = 0; i < GC_IO_STATE_BUFFER_DEPTH; i++ )
			{
				for ( int i2 = 0; i2 < GC_IO_AI_COUNT; i2++ )
				{
					_dest[i][i2] = this->adc_cache[i][i2];
				}
			}
		}

		void BOARD_STATE_CACHE::get_do_cache( DO_CACHE_ENTRY( & _dest ) [GC_IO_STATE_BUFFER_DEPTH] )
		{
			for ( int i = 0; i < GC_IO_STATE_BUFFER_DEPTH; i++ )
			{
				_dest[i] = this->do_cache[i];
			}
		}

		void BOARD_STATE_CACHE::get_pmic_cache( PMIC_CACHE_ENTRY( & _dest ) [GC_IO_STATE_BUFFER_DEPTH] )
		{
			for ( int i = 0; i < GC_IO_STATE_BUFFER_DEPTH; i++ )
			{
				_dest[i] = this->pmic_cache[i];
			}
		}

		void BOARD_STATE_CACHE::get_latest_adc_values( ADC_CACHE_ENTRY( & _dest ) [GC_IO_AI_COUNT] )
		{
			size_t index = 0;

			if ( this->adc_cache_index == 0 )
			{
				index = GC_IO_STATE_BUFFER_DEPTH - 1;
			}
			else
			{
				index = 0;
			}

			for ( size_t i = 0; i < GC_IO_AI_COUNT; i++ )
			{
				_dest[i] = this->adc_cache[index][i];
			}

			return;
		}

		void BOARD_STATE_CACHE::get_latest_do_status( DO_CACHE_ENTRY& _dest )
		{
			if ( this->do_cache_index == 0 )
			{
				_dest = this->do_cache[GC_IO_STATE_BUFFER_DEPTH - 1 ];
			}
			else
			{
				_dest = this->do_cache[this->do_cache_index - 1];
			}

			return;
		}

		void BOARD_STATE_CACHE::get_latest_pmic_status( PMIC_CACHE_ENTRY& _dest )
		{
			if ( this->pmic_cache_index == 0 )
			{
				_dest = this->pmic_cache[GC_IO_STATE_BUFFER_DEPTH - 1 ];
			}
			else
			{
				_dest = this->pmic_cache[this->pmic_cache_index - 1];
			}

			return;
		}

		uint16_t BOARD_STATE_CACHE::get_boot_count( void ) const
		{
			return this->boot_count;
		}

		void BOARD_STATE_CACHE::set_boot_count( uint16_t _value )
		{
			this->boot_count = _value;
		}

		void BOARD_STATE_CACHE::get_latest_l1_cal_values( CAL_VALUE_ENTRY( & _dest ) [GC_IO_AI_COUNT] ) const
		{
			size_t index = 0;

			if ( this->l1_cal_cache_index == 0 )
			{
				index = GC_IO_STATE_BUFFER_DEPTH - 1;
			}
			else
			{
				index = 0;
			}

			for ( size_t i = 0; i < GC_IO_AI_COUNT; i++ )
			{
				_dest[i] = this->cal_l1_cache[index][i];
			}
		}
		void BOARD_STATE_CACHE::get_latest_l2_cal_values( CAL_VALUE_ENTRY( & _dest ) [GC_IO_AI_COUNT] ) const
		{
			size_t index = 0;

			if ( this->l2_cal_cache_index == 0 )
			{
				index = GC_IO_STATE_BUFFER_DEPTH - 1;
			}
			else
			{
				index = 0;
			}

			for ( size_t i = 0; i < GC_IO_AI_COUNT; i++ )
			{
				_dest[i] = this->cal_l2_cache[index][i];
			}
		}
	}
}