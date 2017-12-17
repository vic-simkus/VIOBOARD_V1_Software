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

#ifndef SRC_INCLUDE_LIB_LOGIC_HPP_
#define SRC_INCLUDE_LIB_LOGIC_HPP_

#include <pthread.h>

#include "lib/logger.hpp"
#include "lib/config.hpp"
#include "lib/hvac_types.hpp"
#include "lib/threads/thread_base.hpp"

#include <vector>

#include <string.h>
#include <string>

namespace BBB_HVAC
{
	/**
	 * Class encapsulating the in-time status of the logic core.  Contains things such as digital pin status, DAC values, etc.
	 */
	class LOGIC_STATUS_CORE
	{
	public:
		/**
		 * Digital state typedef
		 */
		typedef bool DIGITAL_STATE;

		/**
		 * Analog state typedef
		 */
		typedef double ANALOG_STATE;

		/**
		 * Digital input state buffer.
		 */
		DIGITAL_STATE* di_buffer;

		/**
		 * Digital output state buffer
		 */
		DIGITAL_STATE* do_buffer;

		/**
		 * Analog input state buffer
		 */
		ANALOG_STATE* ai_buffer;

		/**
		 * Analog output state buffer.
		 */
		ANALOG_STATE* ao_buffer;

		ANALOG_STATE* sp_buffer;

		/**
		 * Number of digital inputs utilized by the logic core.
		 */
		size_t di_num;

		/**
		 * Number of digital outputs utilized by the logic core.
		 */
		size_t do_num;

		/**
		 * Number of analog inputs utilized by the logic core.
		 */
		size_t ai_num;

		/**
		 * Number of analog outputs utilized by the logic core.
		 */
		size_t ao_num;

		size_t sp_num;

		/**
		 * Number of iterations the logic core has performed.  Incremented every time the logic core goes through its processing loop.
		 */
		unsigned long iterations;
		/**
		 * Constructor.
		 */
		inline LOGIC_STATUS_CORE() {
			this->di_buffer = nullptr;
			this->do_buffer = nullptr;
			this->ai_buffer = nullptr;
			this->ao_buffer = nullptr;
			this->sp_buffer = nullptr;
			this->di_num = 0;
			this->do_num = 0;
			this->ai_num = 0;
			this->ao_num = 0;
			this->sp_num = 0;
			this->iterations = 0;
			this->init();
		}

		/**
		 * Copy constructor.
		 */
		inline LOGIC_STATUS_CORE( const LOGIC_STATUS_CORE& _src ) {
			this->di_buffer = nullptr;
			this->do_buffer = nullptr;
			this->ai_buffer = nullptr;
			this->ao_buffer = nullptr;
			this->sp_buffer = nullptr;
			this->init();
			memcpy( this->di_buffer, _src.di_buffer, sizeof( DIGITAL_STATE[GC_LOGIC_DIGITAL_INPUTS] ) );
			memcpy( this->do_buffer, _src.do_buffer, sizeof( DIGITAL_STATE[GC_LOGIC_DIGITAL_OUTPUTS] ) );
			memcpy( this->ai_buffer, _src.ai_buffer, sizeof( ANALOG_STATE[GC_LOGIC_ANALOG_INPUTS] ) );
			memcpy( this->ao_buffer, _src.ao_buffer, sizeof( ANALOG_STATE[GC_LOGIC_ANALOG_OUTPUTS] ) );
			memcpy( this->sp_buffer, _src.sp_buffer, sizeof( ANALOG_STATE[GC_LOGIC_ANALOG_OUTPUTS * 2] ) );
			this->di_num = _src.di_num;
			this->do_num = _src.do_num;
			this->ai_num = _src.ai_num;
			this->ao_num = _src.ao_num;
			this->sp_num = _src.sp_num;
			this->iterations = _src.iterations;
			return;
		}

		/**
		 * Destructor.
		 */
		inline ~LOGIC_STATUS_CORE() {
			delete [] this->di_buffer;
			delete [] this->do_buffer;
			delete [] this->ai_buffer;
			delete [] this->ao_buffer;
			delete [] this->sp_buffer;
			this->di_buffer = nullptr;
			this->do_buffer = nullptr;
			this->ai_buffer = nullptr;
			this->ao_buffer = nullptr;
			this->sp_buffer = nullptr;
			return;
		}

		/**
		 * Returns string representation of the instance intended for human consumption.  Used for debugging purposes.
		 */
		string to_string( void );

	protected:
		/**
		 * Init method.
		 */
		inline void init( void ) {
			this->di_buffer = new DIGITAL_STATE[GC_LOGIC_DIGITAL_INPUTS];
			this->do_buffer = new DIGITAL_STATE[GC_LOGIC_DIGITAL_OUTPUTS];
			this->ai_buffer = new ANALOG_STATE[GC_LOGIC_ANALOG_INPUTS];
			this->ao_buffer = new ANALOG_STATE[GC_LOGIC_ANALOG_OUTPUTS];
			this->sp_buffer = new ANALOG_STATE[GC_LOGIC_ANALOG_OUTPUTS * 2];
			memset( this->di_buffer, 0, sizeof( DIGITAL_STATE[GC_LOGIC_DIGITAL_INPUTS] ) );
			memset( this->do_buffer, 0, sizeof( DIGITAL_STATE[GC_LOGIC_DIGITAL_OUTPUTS] ) );
			memset( this->ai_buffer, 0, sizeof( ANALOG_STATE[GC_LOGIC_ANALOG_INPUTS] ) );
			memset( this->ao_buffer, 0, sizeof( ANALOG_STATE[GC_LOGIC_ANALOG_OUTPUTS] ) );
			memset( this->sp_buffer, 0, sizeof( ANALOG_STATE[GC_LOGIC_ANALOG_OUTPUTS * 2] ) );
		}

	};

	class LOGIC_STATUS_FLUFF
	{
	public:
		inline LOGIC_STATUS_FLUFF() {
			return;
		}

		inline LOGIC_STATUS_FLUFF( const LOGIC_STATUS_FLUFF& _src ) {
			this->di_labels = _src.di_labels;
			this->do_labels = _src.do_labels;
			this->ai_labels = _src.ai_labels;
			this->ao_labels = _src.ao_labels;
			this->sp_labels = _src.sp_labels;
			return;
		}

		LABEL_LIST di_labels;
		LABEL_LIST do_labels;
		LABEL_LIST ai_labels;
		LABEL_LIST ao_labels;
		LABEL_LIST sp_labels;

	};

	/**
	 * Forward declaration
	 */
	class CONFIGURATOR;
	/**
	 * Base class for a logic processor.
	 */
	class LOGIC_PROCESSOR_BASE: public THREAD_BASE
	{

	public:

		/**
		 * Logger instance.
		 */
		LOGGING::LOGGER* logger;

		/**
		 * Constructor.
		 */
		LOGIC_PROCESSOR_BASE( CONFIGURATOR* _config );

		/**
		 * Destructor.
		 */
		virtual ~LOGIC_PROCESSOR_BASE();

		/**
		 * Pure virtual method that is invoked by the owning thread.  Within this method is where the in-time logic processing is done.  This method will be called repeatedly by the owning thread.
		 */
		virtual void process_logic( void ) throw( exception ) = 0;

		/**
		 * Pure virtual method that is invoked by the owning thread.  This method will be exactly called once, by the owning thread, before process_logic.
		 */
		virtual void pre_process( void ) throw( exception ) = 0;

		/**
		 * Pure virtual method that is invoked by the owning thread.  This method will be called exactly once, by the owning thread, after process_logic.
		 */
		virtual void post_process( void ) throw( exception ) = 0;

		/**
		 * Returns a copy of the in-time status of the logic core.
		 * \return A copy of the logic core status.
		 */
		LOGIC_STATUS_CORE get_logic_status( void );

		LOGIC_STATUS_FLUFF get_logic_status_fluff( void ) const;

	protected:

		bool thread_func( void );


		/**
		 * Logic status instance.
		 */
		LOGIC_STATUS_CORE logic_status_core;

		LOGIC_STATUS_FLUFF logic_status_fluff;

		CONFIGURATOR* configurator;

	private:

	};
}

#endif /* SRC_INCLUDE_LIB_LOGIC_HPP_ */
