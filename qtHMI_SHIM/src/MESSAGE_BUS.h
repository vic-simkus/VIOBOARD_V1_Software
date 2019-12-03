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
#ifndef MESSAGE_BUS_H
#define MESSAGE_BUS_H

#include <QObject>
#include <QQueue>
#include <QVector>
#include <QMap>
#include <QPair>
#include <QVariant>

#include "lib/bbb_hvac.hpp"

class QTimer;


/**
	Serves as the main communication interface between the application and the logic core instance.
	Additionally, this class acts a message broker between various parts of the program allowing them to be decoupled from each other.
	All communication is done asynchronously.

	The idea is that all UI elements in an application should be able to share a single connection with the logic core.  Additionally, each UI element should be able to publish its state and its associated changes to interested parties

	A medium term goal is to split this out of the HMI client and into it's own library to facilitate higher level communications.

	Once a second an automatic update is performed with the following commands: COMMANDS::GET_LOGIC_STATUS, COMMANDS::GET_SET_POINTS.
	should be decoupled from each other.
*/
class MESSAGE_BUS : public QObject
{
		Q_OBJECT;
	public:
		/**
		Supported commands
		*/
		enum class COMMANDS : unsigned int
		{
			/// No command
			NONE = 0,
			/// Gets the DO (digital output) labels
			GET_LABELS_DO,
			/// Gets the AI (analog input) labels
			GET_LABELS_AI,
			/// Gets the logic core map
			GET_LABELS_MAP,
			/// Gets the set points
			GET_SET_POINTS,
			/// Gets the logic status
			GET_LOGIC_STATUS,
			/// Gets the status of all analog inputs, digital outputs, and PMIC status
			GET_STATUS,
			/// Sets the calibration values on a board
			SET_CAL_VALS,
			// Sets the DO output status on a board
			SET_DO,
			// Sets the PMIC status on a board
			SET_PMIC,
			FORCE_AI_VALUE,
			UNFORCE_AI_VALUE
		};

		/// Message for
		class MESSAGE
		{
			public:
				MESSAGE( const QString& _board_id, COMMANDS _command, const QVariant& _payload );
				MESSAGE( const QString& _board_id, COMMANDS _command );
				MESSAGE( COMMANDS _command, const QVariant& _payload );
				MESSAGE( COMMANDS _command );

				QString board_id;
				COMMANDS command;
				QVariant payload;

			public:
				static MESSAGE create_message_set_cal_vals( const QString& _board_id, const QVector<uint16_t>& _cal_val_l1, const QVector<uint16_t>& _cal_val_l2 );
				static MESSAGE create_message_set_do_status( const QString& _board_id, const uint8_t _do_status );
				static MESSAGE create_message_set_pmic_status( const QString& _board_id, const uint8_t _pmic_status );
				static MESSAGE create_message_force_ai_value( const QString& _board_id, const uint8_t _ai_idx, const uint16_t _value );
				static MESSAGE create_message_unforce_ai_value( const QString& _board_id, const uint8_t _ai_idx );
		};

		/**
		Constructor
		\param _update_frequency The number of times per second that the message queue will be processed.
		*/
		MESSAGE_BUS( uint8_t _update_frequency );

		/**
		Destructor
		*/
		~MESSAGE_BUS();

		/**
		Adds a message to the message queue.
		*/
		void add_message( const MESSAGE& _message );

		/**
		Returns connection status
		\return true - if there is an active connection to a remove logic core.
		*/
		bool is_connected( void ) const;

	public slots:
		/**
		Allows a UI element to publish a change event.  This method does no processing of the data.  It emits sig_raw_adc_value_changed
		\param _board board tag.
		\param _io analog input index.
		\param _value new value
		*/
		void slot_raw_adc_value_changed( const QString& _board, uint8_t _io, uint16_t _value );

	signals:
		void sig_get_status( const QString& _board, const QVector<uint16_t>& _adc_values, const QVector<bool>& _do_states, bool _pmic_do_en, bool _pmic_do_fault, bool _pmic_ai_en, bool _pmic_ai_fault, const QVector<uint16_t>& _cal_vals_l1, const QVector<uint16_t>& _cal_vals_l2 );
		/**
		Is emitted every time the slot_raw_adc_value_change is invoked.
		\param _board board taq.
		\param _io analog input index.
		\param _value new value
		*/
		void sig_raw_adc_value_changed( const QString& _board, uint8_t _io, uint16_t _value );

		/**
		Emitted as a response to the COMMANDS::GET_LABELS_DO message
		\param _cmd command
		\param _data a vector of vectors.  Each vector consists of the following three elements: 0 - boar tag, 1 - point ID within the context of the board, 2 - description.
		*/
		void sig_point_label_data( MESSAGE_BUS::COMMANDS _cmd, const QVector<QVector<QString>>& _data );

		/**
		Emitted as a response to the COMMANDS::GET_LABELS_MAP message
		\param _cmd command
		\param _data a map of vectors.  Map key is the map name i.e. the name by which logic core accesses the point.  Vector: 0 - board tag, 1 - point type, 2 - point ID within the context of the board, 3 - description
		*/
		void sig_point_map_data( MESSAGE_BUS::COMMANDS  _cmd, const QMap<QString, QVector<QString>>& _data );

		/*
		Emitted as a response to the COMMANDS::GET_LOGIC_STATUS command.
		\param _cmd command
		\param _data a map of strings.  Map key is the map name (from COMMANDS::GET_LABELS_MAP) and map value is the logic status value.
		*/
		void sig_logic_status_data( MESSAGE_BUS::COMMANDS _cmd, const QMap<QString, QString>&  _data );

		/**
		Emitted as a response to the COMMANDS::GET_SET_POINTS command.
		\param _cmd command
		\param _data a map of strings.  Map key is the set point name and map value is the set point value.
		*/
		void sig_set_point_data( MESSAGE_BUS::COMMANDS  _cmd, const QMap<QString, QString>&  _data );

		/**
		Emitted every time (once a second) an automatic update is started.
		*/
		void sig_update_started();

		/**
		Emitted very time (once a second) an automatic update is finished.
		*/
		void sig_update_finished();
	protected:
		/**
		Processes the enqueued messages.  Invoked by do_update every time do_update is invoked.
		\see do_update
		*/
		void process_commands( void );

		/**
		Performs an automatic update of the various values once per second.
		*/
		void perform_major_update();

		void emit_get_status_message( const MESSAGE& _message , const BBB_HVAC::MESSAGE_PTR& _data );

		/**
		Emits a point label inquiry result message.
		*/
		void emit_point_label_message( COMMANDS _command, const BBB_HVAC::MESSAGE_PTR& _data );

		/**
		Emits a map data inquiry result message
		*/
		void emit_map_data_mesage( COMMANDS _command, const BBB_HVAC::MESSAGE_PTR& _data );

		/**
		Emits a logic status inquiry results message
		*/
		void emit_logic_status_update_message( COMMANDS _command, const BBB_HVAC::MESSAGE_PTR& _data );

		/**
		Emits a poind data inquiry results message
		*/
		void emit_set_point_data_message( COMMANDS _command, const  BBB_HVAC::MESSAGE_PTR& _data );

		void set_cal_vals( const MESSAGE_BUS::MESSAGE& _message );

		void force_value( const MESSAGE_BUS::MESSAGE& _message );
		void unforce_value( const MESSAGE_BUS::MESSAGE& _message );
	protected slots:
		/**
		Invoked by the timer every (1000/update_frequenct) milliseconds.
		*/
		void do_update( void );

	private:
		/// The command queue.  Commands are added by the add_message method and consumed periodically by the do_update method.
		QQueue<MESSAGE> command_queue;

		/// Connection to the remove logic core.
		BBB_HVAC::CLIENT::CLIENT_CONTEXT* ctx;

		/// The timer that drives the update logic
		QTimer* timer_update;

		/// Number of times the do_update method has been invoked.
		uint8_t update_counter;

		/// How many times per second an update should be performed.
		uint8_t update_frequency;

		/// How many times we've failed to connect to the logic core.
		uint8_t failure_count;
};

#endif