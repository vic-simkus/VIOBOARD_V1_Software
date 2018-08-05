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

#include "lib/exceptions.hpp"
#include "lib/config.hpp"
#include "lib/globals.hpp"
#include "lib/string_lib.hpp"
#include "lib/configurator.hpp"
#include "lib/threads/logic_thread.hpp"

#include "lib/threads/watchdog_thread.hpp"
#include "lib/threads/thread_registry.hpp"

#include "lib/serial_io_types.hpp"

#include <signal.h>
#include <string.h>
#include <algorithm>
#include <iterator>

using namespace BBB_HVAC;

LOGIC_PROCESSOR_BASE::LOGIC_PROCESSOR_BASE( CONFIGURATOR* _config ) :
	THREAD_BASE( "LOGIC_PROCESSOR_BASE" )
{
	this->configurator = _config;
	this->logger = new LOGGING::LOGGER( "BBB_HVAC::LOGIC_PROCESSOR_BASE" );
	/*
	Populate the logic fluff stuff.  Logic fluff is used by user-facing stuffs to extract operational information out of the logic processor
	*/
	this->logic_status_fluff.do_labels = this->configurator->get_do_points();
	this->logic_status_fluff.ai_labels = this->configurator->get_ai_points();
	this->logic_status_fluff.sp_labels = this->configurator->get_sp_points();
	this->logic_status_fluff.point_map = this->configurator->get_point_map();

	for( auto i = this->logic_status_fluff.do_labels.cbegin(); i != this->logic_status_fluff.do_labels.cend(); ++i )
	{
		std::string board_tag = ( *i ).get_board_tag();

		if( std::find( this->involved_board_tags.cbegin(), this->involved_board_tags.cend(), board_tag ) == this->involved_board_tags.cend() )
		{
			this->involved_board_tags.push_back( board_tag );
		}
	}

	for( auto i = this->logic_status_fluff.ai_labels.cbegin(); i != this->logic_status_fluff.ai_labels.cend(); ++i )
	{
		std::string board_tag = ( *i ).get_board_tag();

		if( std::find( this->involved_board_tags.cbegin(), this->involved_board_tags.cend(), board_tag ) == this->involved_board_tags.cend() )
		{
			this->involved_board_tags.push_back( board_tag );
		}
	}

	for( auto i = this->involved_board_tags.cbegin(); i != this->involved_board_tags.cend(); ++i )
	{
		this->logic_status_core.current_state_map.emplace( std::make_pair( *i, BOARD_STATE_STRUCT() ) );
	}

	LOG_DEBUG_P( "adc_vref_max: " + num_to_str( this->logic_status_core.adc_vref_max ) );
	LOG_DEBUG_P( "adc_steps: " + num_to_str( this->logic_status_core.adc_steps ) );
	LOG_DEBUG_P( "adc_step_val: " + num_to_str( this->logic_status_core.adc_step_val ) );
	return;
}

LOGIC_PROCESSOR_BASE::~LOGIC_PROCESSOR_BASE()
{
	delete this->configurator;
	this->configurator = nullptr;
	delete this->logger;
	this->logger = nullptr;
	return;
}

std::map<std::string, LOGIC_POINT_STATUS> LOGIC_PROCESSOR_BASE::get_logic_status( void )
{
	std::map<std::string, LOGIC_POINT_STATUS> ret;
	this->obtain_lock();

	for( auto map_iterator = this->configurator->get_point_map().cbegin(); map_iterator != this->configurator->get_point_map().cend(); ++map_iterator )
	{
		const std::string map_name = map_iterator->first;
		const BOARD_POINT& board_point = map_iterator->second;
		ENUM_CONFIG_TYPES point_type = board_point.get_type();

		if( point_type == ENUM_CONFIG_TYPES::DO )
		{
			// Digital output point
			bool value = ( this->logic_status_core.current_state_map.at( board_point.get_board_tag() ).do_state.get_value() & ( 1 << board_point.get_point_id() ) ) ? true : false;
			ret.emplace( std::make_pair( map_name, LOGIC_POINT_STATUS( value ) ) );
		}
		else if( point_type == ENUM_CONFIG_TYPES::AI )
		{
			// Analog input
			ret.emplace( std::make_pair( map_name, LOGIC_POINT_STATUS( this->logic_status_core.calculated_adc_values[map_name] ) ) );
		}
		else
		{
			LOG_ERROR_P( "Unrecognized point type in point map: " + num_to_str( static_cast<unsigned int>( point_type ) ) );
		}
	}

	this->release_lock();
	return ret;
}

void LOGIC_PROCESSOR_BASE::get_logic_status_fluff( LOGIC_STATUS_FLUFF& _dest ) const
{
	_dest = this->logic_status_fluff;
}

string LOGIC_STATUS_CORE::to_string( void )
{
	string ret;
	return ret;
}

bool LOGIC_PROCESSOR_BASE::inner_thread_func( void )
{
	LOG_INFO_P( "Starting logic thread." );
	this->obtain_lock();
	this->pre_process();
	this->release_lock();

	while( this->abort_thread == false )
	{
		this->reset_sleep_timespec( GC_LOGIC_THREAD_SLEEP );
		this->nsleep();
		GLOBALS::watchdog->reset_counter();
		this->obtain_lock();

		/*
		 At this point we have the mutex lock.

		 Before the subclasses process method is call we get the latest board statuses and precalculate all of the analog input values.

		Step 1 of 2: Get all of the current IO board values.
		*/
		for( auto i = this->involved_board_tags.cbegin(); i != this->involved_board_tags.cend(); ++i )
		{
			/*
			involved_board_tags is a vector of strings - board IDs
			*/
			/*
			The boards IO thread handle.
			*/
			IOCOMM::SER_IO_COMM* thread_handle;

			try
			{
				thread_handle = THREAD_REGISTRY::get_serial_io_thread( *i );
			}
			catch( const exception& _e )
			{
				LOG_ERROR_P( "Failed to find board IO thread: (" + std::string( _e.what() ) + ").  Skipping logic iteration." );
				goto _end;
			}

			try
			{
				/*
				Get the board state for the current board.
				*i is the board name.
				*/
				auto board_state_iterator = this->logic_status_core.current_state_map.find( ( *i ) );

				if( board_state_iterator == this->logic_status_core.current_state_map.end() )
				{
					LOG_ERROR_P( "Failed to find state map for the current board tag: " + ( *i ) );
					goto _end;
				}

				BOARD_STATE_STRUCT* board_state_ptr = &board_state_iterator->second;
				BBB_HVAC::IOCOMM::BOARD_STATE_CACHE board_state_cache;
				thread_handle->get_latest_state_values( board_state_cache );
				board_state_cache.get_latest_do_status( board_state_ptr->do_state );
				board_state_cache.get_latest_do_status( board_state_ptr->do_state );
				board_state_cache.get_latest_pmic_status( board_state_ptr->pmic_state );
				board_state_cache.get_latest_adc_values( board_state_ptr->ai_state );
			}
			catch( const exception& _e )
			{
				LOG_ERROR_P( "Failed to update state: " + string( _e.what() ) );
			}
		}

		/*
		Step 2 of 2: Precalculate the analog input values.
		*/
		const auto&  point_map = this->configurator->get_point_map();

		for( auto map_iterator = point_map.cbegin(); map_iterator != point_map.cend(); ++map_iterator )
		{
			const std::string& point_name = map_iterator->first;
			const BOARD_POINT& board_point = map_iterator->second;
			const auto& board_state_iterator = this->logic_status_core.current_state_map.find( board_point.get_board_tag() );
			const BOARD_STATE_STRUCT& board_state = board_state_iterator->second;
			uint16_t val = board_state.ai_state[board_point.get_point_id()].get_value();
			double volt_value = ( double )val * this->logic_status_core.adc_step_val;
			double calculated_value = 0;

			if( board_point.get_ai_type() == AI_TYPE::CL_420 )
			{
				calculated_value = calculate_420_value( volt_value, board_point.get_min_value(), board_point.get_max_value() );
			}
			else if( board_point.get_ai_type() == AI_TYPE::ICTD )
			{
				// TODO - calculate ICTD temp value.
				double in_volts = volt_value / 10;	// The opamp gain is 10x
				calculated_value = calculate_ICTD_value( in_volts );
				calculated_value = board_point.get_is_celcius() ? calculated_value : c_to_f( calculated_value );
				//LOG_DEBUG_P("Defrees C: " + num_to_str(calculated_value) + " F: " + num_to_str(c_to_f(calculated_value)));
			}

			this->logic_status_core.calculated_adc_values[point_name] = calculated_value;
		}

		/*
		Update the set point values in the logic core status

		XXX -- And we're doing this because ... the user might change the setpoints???
		*/
		const SET_POINT_MAP& set_points = this->configurator->get_sp_points();

		for( auto i = set_points.begin(); i != set_points.end(); ++i )
		{
			this->logic_status_core.set_point_values[i->first] = this->configurator->get_sp_value( i->first );
		}

		try
		{
			this->process_logic();
		}
		catch( const std::exception& e )
		{
			LOG_ERROR_P( "process_logic() emitted an std::exception.  Logic thread will abort. Message: " + std::string( e.what() ) );
			this->abort_thread = true;
		}
		catch( ... )
		{
			LOG_ERROR_P( "process_logic() emitted an unspecified exception.  Logic thread will abort." );
			this->abort_thread = true;
		}

		/*
		 * Done with the logic processing.  Unlock the mutex so that some other thread may get the status.
		 */
		this->release_lock();
	}

_end:
	/*
	 * If we end up here there's two possibilities:
	 * 1 -- The user decided to shut the daemon down
	 * 2 -- Something went to shit.
	 *
	 * Point being, the logic_core is an indeterminate state and the post_process call is merely informational.
	 */
	this->post_process();
	GLOBALS::global_exit_flag = true;
	LOG_INFO_P( "Logic thread finished." );
	return true;
}

bool LOGIC_PROCESSOR_BASE::thread_func( void )
{
	try
	{
		return this->inner_thread_func();
	}
	catch( const exception& _e )
	{
		LOG_ERROR_P( "Unhandled exception caught in the main logic loop: " + string( _e.what() ) );
	}
	catch( ... )
	{
		LOG_ERROR_P( "Unhandled and unknown exception caught in the main logic loop." );
	}

	try
	{
		this->release_lock();
	}
	catch( const exception& _e )
	{
		LOG_ERROR_P( "Ignoring exception during last ditch effort to release lock: " + string( _e.what() ) );
	}

	LOG_ERROR_P( "Aborting application." );
	GLOBALS::global_exit_flag = true;
	return false;
}

double LOGIC_PROCESSOR_BASE::c_to_f( double _c )
{
	return( ( _c * 9 / 5 ) + 32 );
}
double LOGIC_PROCESSOR_BASE::calculate_ICTD_value( double _voltage )
{
	/*
	The voltage we're expecting here is the voltage at the terminal not at the opamp output.

	The ICTD terminates into a 1K resistor.  By ohms law I = V / R.  For an input of lets say 0.333V, I = 0.000333A.
	We're interested in uA since 1 uA = 1 Kelvin.  uA = I * 1,000,000.
	So instead of doing a divide by 1000 and multiply by 1,000,000 we just multiply by 1,000
	*/
	double milliamps = _voltage * 1000;
	double degrees_c = milliamps - ( double )273.15;
	return degrees_c;
}
double LOGIC_PROCESSOR_BASE::calculate_420_value( double _voltage, long _min, long _max )
{
	/*
	XXX Bit of a fuckup here.  We're assuming that the sensor is going into a 250 ohm resistor.

	x1000 in order to get milliamp value

	*/
	double current = ( ( _voltage / ( double ) 250 ) * 1000 );
	double value  = 0;
	long min_max_delta = _max - _min;
	const double i_delta = 20 - 4;		// durrr 16
	value = ( ( double )min_max_delta / i_delta ) * ( current - 4 ) + ( double )_min;
	return value;
}

double LOGIC_PROCESSOR_BASE::get_sp_value( const string& _name ) const throw( exception )
{
	return this->logic_status_core.set_point_values.at( _name );
}
void LOGIC_PROCESSOR_BASE::set_sp_value( const string& _name, double _value )
{
	this->configurator->set_sp_value( _name, _value );
}

double LOGIC_PROCESSOR_BASE::get_ai_value( const string& _name ) const
{
	return this->logic_status_core.calculated_adc_values.at( _name );
}

bool LOGIC_PROCESSOR_BASE::is_output_set( const string& _name ) const
{
	const BOARD_POINT& board_point = this->configurator->get_point_map().at( _name );
	uint8_t do_status = this->logic_status_core.current_state_map.at( board_point.get_board_tag() ).do_state.get_value();

	if( do_status & ( 1 << board_point.get_point_id() ) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void LOGIC_PROCESSOR_BASE::set_output( const string& _name )
{
	BOARD_POINT board_point = this->configurator->get_point_map().at( _name );
	uint8_t do_status = this->logic_status_core.current_state_map.at( board_point.get_board_tag() ).do_state.get_value();

	if( do_status & ( 1 << board_point.get_point_id() ) )
	{
		// Do nothing.  The DO is already set.
		return;
	}

	IOCOMM::SER_IO_COMM* thread_handle = THREAD_REGISTRY::get_serial_io_thread( board_point.get_board_tag() );
	uint8_t v = ( uint8_t )( 1 << board_point.get_point_id() );
	do_status |= v;
	thread_handle->cmd_set_do_status( do_status );
	return;
}
void LOGIC_PROCESSOR_BASE::clear_output( const string& _name )
{
	BOARD_POINT p = this->configurator->get_point_map().at( _name );
	/*
	TODO - need to implement
	*/
}