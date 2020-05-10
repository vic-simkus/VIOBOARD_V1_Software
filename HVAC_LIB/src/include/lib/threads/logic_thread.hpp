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
#include "lib/configurator.hpp"
#include "lib/serial_io_types.hpp"

#include <vector>
#include <map>
#include <string.h>
#include <string>
#include <time.h>


namespace BBB_HVAC
{
	typedef struct
	{
		unsigned long long int last_reset;
		unsigned int count;
		bool failed;
	} PMIC_RESET;

	typedef struct
	{
		IOCOMM::DO_CACHE_ENTRY do_state;
		IOCOMM::PMIC_CACHE_ENTRY pmic_state;
		IOCOMM::ADC_CACHE_ENTRY ai_state[GC_IO_AI_COUNT];
		std::string board_tag;
	} BOARD_STATE_STRUCT;

	class LOGIC_POINT_STATUS
	{
		public:
			explicit inline LOGIC_POINT_STATUS( double _double_value ) {
				this->is_double_value = true;
				this->double_value = _double_value;
				this->bool_value = false;
				return;
			}

			explicit inline LOGIC_POINT_STATUS( bool _bool_value ) {
				this->is_double_value = false;
				this->double_value = 0;
				this->bool_value = _bool_value;
				return;
			}

			bool is_double_value;
			double double_value;
			bool bool_value;
	};

	/**
	 * Class encapsulating the in-time status of the logic core.  Contains things such as digital pin status, DAC values, etc.
	 */
	class LOGIC_STATUS_CORE
	{
		public:


			/**
			 * Number of iterations the logic core has performed.  Incremented every time the logic core goes through its processing loop.
			 */
			unsigned long iterations;

			/**
			Boars state map that we generate from the serial IO thread's state cache.
			The key is the board tag.  Value is an instance of BOARD_STATE_STRUCT.
			*/
			std::map<std::string, BOARD_STATE_STRUCT> current_state_map;

			/**
			A map of setpoint values.  Key is setpoint name.  Value is ... the value.
			*/
			std::map<std::string, double> set_point_values;

			/**
			A map of analog input values.  The key is the globally unique point name from the configuration MAP statement.  The value is the calculated value of the analog input based on the point's configuration.
			*/
			std::map<std::string, double> calculated_adc_values;

			double adc_vref_max;
			double adc_step_val;
			unsigned int adc_steps;

			/**
			 * Constructor.
			 */
			inline LOGIC_STATUS_CORE() {
				this->init();
			}


			/**
			 * Destructor.
			 */
			inline ~LOGIC_STATUS_CORE() {
				this->iterations = 0;
				this->adc_vref_max = 0;
				this->adc_steps = 0;
				this->adc_step_val = 0;
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
				this->iterations = 0;
				this->adc_vref_max = GC_IO_ADC_VREF_MAX;
				this->adc_steps = GC_IO_ADC_STEPS;
				this->adc_step_val = this->adc_vref_max / this->adc_steps;
				return;
			}
		private:
			/**
			 * Copy constructor.
			 */
			inline LOGIC_STATUS_CORE( const LOGIC_STATUS_CORE& ) {
				throw logic_error( "Can not copy LOGIC_STATUS_CORE" );
				return;
			}
	};

	class LOGIC_STATUS_FLUFF
	{
		public:
			inline LOGIC_STATUS_FLUFF() {
				return;
			}

			inline LOGIC_STATUS_FLUFF( const LOGIC_STATUS_FLUFF& _src ) {
				this->do_labels = _src.do_labels;
				this->ai_labels = _src.ai_labels;
				this->sp_labels = _src.sp_labels;
				this->point_map = _src.point_map;
				return;
			}

			BOARD_POINT_VECTOR do_labels;
			BOARD_POINT_VECTOR ai_labels;
			SET_POINT_MAP sp_labels;
			std::map<std::string, BOARD_POINT> point_map;
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
			virtual void process_logic( void )  = 0;

			/**
			 * Pure virtual method that is invoked by the owning thread.  This method will be exactly called once, by the owning thread, before process_logic.
			 */
			virtual void pre_process( void )  = 0;

			/**
			 * Pure virtual method that is invoked by the owning thread.  This method will be called exactly once, by the owning thread, after process_logic.
			 */
			virtual void post_process( void )  = 0;

			/**
			 * Returns a copy of the in-time status of the logic core.
			 * \return A copy of the logic core status.
			 */
			std::map<std::string, LOGIC_POINT_STATUS> get_logic_status( void );

			void get_logic_status_fluff( LOGIC_STATUS_FLUFF& ) const;

			void set_sp_value( const string& _name, double _value ) ;

		protected:
			static double calculate_420_value( double _voltage, long _min, long _max );
			static double calculate_ICTD_value( double _voltage );
			static double c_to_f( double c );

			bool thread_func( void );

			bool inner_thread_func( void );


			/**
			\note This method does not acquire the thread lock and thus is expected to only be used once the lock has already been acquired.
			*/
			double get_sp_value( const string& _name ) const;

			/**
			\note This method does not acquire the thread lock and thus is expected to only be used once the lock has already been acquired.
			*/
			void set_sp_value_ns( const string& _name, double _value );

			/**
			\note This method does not acquire the thread lock and thus is expected to only be used once the lock has already been acquired.
			*/
			double get_ai_value( const string& _name ) const;

			/**
			\note This method does not acquire the thread lock and thus is expected to only be used once the lock has already been acquired.
			*/
			bool is_output_set( const string& _name ) const;

			/**
			\note This method does not acquire the thread lock and thus is expected to only be used once the lock has already been acquired.
			*/
			void set_output( const string& _name );

			/**
			\note This method does not acquire the thread lock and thus is expected to only be used once the lock has already been acquired.
			*/
			void clear_output( const string& _name );

			/**
			\note This method does not acquire the thread lock and thus is expected to only be used once the lock has already been acquired.
			*/
			//const BOARD_POINT& get_board_point(const string& _name) const;

			/**
			 * Logic status instance.
			 */
			LOGIC_STATUS_CORE logic_status_core;

			LOGIC_STATUS_FLUFF logic_status_fluff;

			CONFIGURATOR* configurator;

			std::vector<std::string> involved_board_tags;

			size_t config_save_counter;

			std::map<std::string, PMIC_RESET> pmic_reset_counters;
		private:

			DEF_LOGGER;

	};
}

#endif /* SRC_INCLUDE_LIB_LOGIC_HPP_ */

