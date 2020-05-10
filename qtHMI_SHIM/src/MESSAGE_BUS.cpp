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

#include "MESSAGE_BUS.h"

#include <QTimer>

using namespace BBB_HVAC;
using namespace BBB_HVAC::CLIENT;

DEF_LOGGER_STAT( "MESSAGE_BUS" );
MESSAGE_BUS::MESSAGE_BUS( uint8_t _update_frequency, SOCKET_TYPE _st, const QString& _address, uint16_t _port ) : QObject()
{
	// The timer that will periodically invoke the message processing logic
	this->timer_update = new QTimer( this );
	this->update_counter = 0;
	this->update_frequency = _update_frequency;
	// On timer fire run the message processing logic
	connect( this->timer_update, SIGNAL( timeout() ), this, SLOT( do_update() ) );
	this->ctx = nullptr;
	this->failure_count = 0;

	this->st = _st;
	this->address = _address;
	this->port = _port;

	return;
}

MESSAGE_BUS::~MESSAGE_BUS()
{
	try
	{
		this->ctx->disconnect();
	}
	catch ( ... )
	{
		//ignore
		LOG_DEBUG( "Disconnection attempt failed." );
	}

	this->ctx = nullptr;
	return;
}

void MESSAGE_BUS::connect_to_remote( void )
{
	/*
		Check if we're connected
		*/
	if ( this->ctx == nullptr )
	{

		// If not, connect.
		this->ctx = BBB_HVAC::CLIENT::CLIENT_CONTEXT::create_instance( this->st, this->address.toStdString(), this->port );

		try
		{
			this->ctx->connect( );
		}
		catch ( const BBB_HVAC::EXCEPTIONS::CONNECTION_ERROR& _e )
		{
			LOG_ERROR( "Failed to connect: " +  std::string( _e.what() ) );
			delete this->ctx;
			this->ctx = nullptr;
			this->failure_count += 1;

			throw std::runtime_error( std::string( "Failed to connect to remote: " ) + _e.what() );
		}

		LOG_DEBUG( "MESSAGE_BUS connected to remote point." );
	}
	else
	{
		throw std::logic_error( "Instance appears to already be connected.  Can not connect twice." );
	}

	this->timer_update->start( 1000 / update_frequency );
}

void MESSAGE_BUS::do_update( void )
{
	/*
	Invoked by the timer this->update_frequency times per second.
	*/

	if ( this->ctx == nullptr )
	{
		throw std::logic_error( "Instance not connected.  Call connect() first." );
	}


	// The various automatic updates are performed once a second
	if ( this->update_counter == this->update_frequency )
	{
		// Indicate to those interested that an update has started.
		this->sig_update_started();
		// Perform the major update
		this->perform_major_update();
		this->update_counter = 0;
		// Indicate to those interested that an update has finished
		this->sig_update_finished();
	}

	// We process commands every invocation
	this->process_commands();
	this->update_counter += 1;
	return;
}

void MESSAGE_BUS::process_commands( void )
{
	// Processes all of the queued commands.

	BBB_HVAC::MESSAGE_PTR message;

	while ( !this->command_queue.isEmpty() )
	{
		MESSAGE_BUS::MESSAGE c = this->command_queue.dequeue();
		//LOG_DEBUG_STAT( "Processing message: " + num_to_str( ( int )c.command ) );

		switch ( c.command )
		{
			case COMMANDS::NONE:
				// Do nothing;
				break;

			case COMMANDS::FORCE_AI_VALUE:
				force_value( c );
				break;

			case COMMANDS::UNFORCE_AI_VALUE:
				unforce_value( c );
				break;

			case COMMANDS::SET_CAL_VALS:
				set_cal_vals( c );
				break;

			case COMMANDS::GET_STATUS:
				message = this->ctx->message_processor->create_get_status( c.board_id.toStdString() );
				message = this->ctx->send_message_and_wait( message );
				this->emit_get_status_message( c, message );
				break;

			case COMMANDS::GET_LABELS_DO:
				message = this->ctx->message_processor->create_get_labels_message_request( BBB_HVAC::ENUM_CONFIG_TYPES::DO );
				message = this->ctx->send_message_and_wait( message );
				this->emit_point_label_message( c.command, message );
				break;

			case COMMANDS::GET_LABELS_AI:
				message = this->ctx->message_processor->create_get_labels_message_request( BBB_HVAC::ENUM_CONFIG_TYPES::AI );
				message = this->ctx->send_message_and_wait( message );
				this->emit_point_label_message( c.command, message );
				break;

			case COMMANDS::GET_LABELS_MAP:
				message = this->ctx->message_processor->create_get_labels_message_request( BBB_HVAC::ENUM_CONFIG_TYPES::MAP );
				message = this->ctx->send_message_and_wait( message );
				this->emit_map_data_mesage( c.command, message );
				break;

			case COMMANDS::GET_SET_POINTS:
				message = this->ctx->message_processor->create_get_labels_message_request( BBB_HVAC::ENUM_CONFIG_TYPES::SP );
				message = this->ctx->send_message_and_wait( message );
				this->emit_set_point_data_message( c.command, message );
				break;

			case COMMANDS::GET_LOGIC_STATUS:
				message = this->ctx->message_processor->create_read_logic_status( );
				message = this->ctx->send_message_and_wait( message );
				this->emit_logic_status_update_message( c.command, message );
				break;

			case COMMANDS::SET_DO:
				message = this->ctx->message_processor->create_set_status( c.board_id.toStdString(), ( uint8_t )c.payload.toUInt() );
				this->ctx->send_message( message );
				break;

			case COMMANDS::SET_PMIC:
				message = this->ctx->message_processor->create_set_pmic_status( c.board_id.toStdString(), ( uint8_t )c.payload.toUInt() );
				this->ctx->send_message( message );
				break;
		};
	}
}

void MESSAGE_BUS::force_value( const MESSAGE_BUS::MESSAGE& _message )
{
	BBB_HVAC::MESSAGE_PTR message;

	uint8_t ai_idx = ( uint8_t )_message.payload.toList().at( 0 ).toUInt();
	uint16_t ai_value = ( uint16_t )_message.payload.toList().at( 1 ).toUInt();

	message = this->ctx->message_processor->create_force_ai( _message.board_id.toStdString(), ai_idx, ai_value );

	//LOG_DEBUG( "Message: " + message->to_string() );

	this->ctx->send_message( message );

	return;
}
void MESSAGE_BUS::unforce_value( const MESSAGE_BUS::MESSAGE& _message )
{
	BBB_HVAC::MESSAGE_PTR message;

	uint8_t ai_idx = ( uint8_t )_message.payload.toUInt();
	message = this->ctx->message_processor->create_unforce_ai( _message.board_id.toStdString(), ai_idx );
	this->ctx->send_message( message );

	return;
}

static void variant_list_to_cal_array( BBB_HVAC::CAL_VALUE_ARRAY& _cal_vals, const QList<QVariant>& _v_l )
{
	_cal_vals.clear();

	for ( auto i = _v_l.begin(); i != _v_l.end(); ++i )
	{
		_cal_vals.push_back( ( *i ).toUInt() );
	}

	return;
}

void MESSAGE_BUS::set_cal_vals( const MESSAGE_BUS::MESSAGE& _message )
{
	BBB_HVAC::MESSAGE_PTR message;

	// This is an std::vector.
	BBB_HVAC::CAL_VALUE_ARRAY calibration_values;
	const QList<QVariant>& payload = _message.payload.toList();

	variant_list_to_cal_array( calibration_values, payload.at( 0 ).toList() );
	// At this point the calibration_values array contains all of the calibration values converted from the various QVariants.
	message = this->ctx->message_processor->create_set_l1_cal_vals( _message.board_id.toStdString(), calibration_values );
	this->ctx->send_message( message );
	//LOG_DEBUG_STAT( "L1 cal vals: " + message->to_string() );

	variant_list_to_cal_array( calibration_values, payload.at( 1 ).toList() );
	message = this->ctx->message_processor->create_set_l2_cal_vals( _message.board_id.toStdString(), calibration_values );
	this->ctx->send_message( message );
	//LOG_DEBUG_STAT( "L2 cal vals: " + message->to_string() );

	return;
}

void MESSAGE_BUS::perform_major_update( void )
{
	// Performs an automatic periodic update

	BBB_HVAC::MESSAGE_PTR message;

	// Do the logic status update
	message = this->ctx->message_processor->create_read_logic_status( );
	message = this->ctx->send_message_and_wait( message );
	this->emit_logic_status_update_message( COMMANDS::GET_LOGIC_STATUS, message );

	// Do the setpoint update.
	message = this->ctx->message_processor->create_get_labels_message_request( BBB_HVAC::ENUM_CONFIG_TYPES::SP );
	message = this->ctx->send_message_and_wait( message );
	this->emit_set_point_data_message( COMMANDS::GET_SET_POINTS, message );

	return;
}

void MESSAGE_BUS::add_message( const MESSAGE& _message )
{
	this->command_queue.enqueue( _message );
	return;
}

void MESSAGE_BUS::slot_raw_adc_value_changed( const QString& _board, uint8_t _io, uint16_t _value )
{
	this->sig_raw_adc_value_changed( _board, _io, _value );
}


void MESSAGE_BUS::emit_get_status_message( const MESSAGE& _message , const BBB_HVAC::MESSAGE_PTR& _data )
{

	//LOG_DEBUG_STAT( "Board: " + _message.board_id.toStdString() );
	//DEF_LOGGER_STAT( "Processing message: " + num_to_str( ( int )c.command ) );
	QVector<uint16_t> adc_values;
	QVector<uint16_t> cal_vals_l1;
	QVector<uint16_t> cal_vals_l2;

	QVector<bool> do_states;

	bool pmic_do_en;
	bool pmic_do_fault;
	bool pmic_ai_en;
	bool pmic_ai_fault;

	adc_values.resize( GC_IO_AI_COUNT );
	cal_vals_l1.resize( GC_IO_AI_COUNT );
	cal_vals_l2.resize( GC_IO_AI_COUNT );

	do_states.resize( GC_IO_DO_COUNT );

	const vector<string>& parts = _data->get_parts( );

	for ( size_t i = 0; i < ( size_t )adc_values.size(); ++i )
	{
		BBB_HVAC::IOCOMM::ADC_CACHE_ENTRY adc_val( parts[i] );
		adc_values[i] = adc_val.get_value();

		/*
		In the message from the logic core, l1 cal values are after the AI_COUNT AI values, DO status and PMIC status.
		l2 cal values are after the l1 cal values.
		*/
		BBB_HVAC::IOCOMM::CAL_VALUE_ENTRY cv_l1( parts[( GC_IO_AI_COUNT + 2 ) + i] );
		BBB_HVAC::IOCOMM::CAL_VALUE_ENTRY cv_l2( parts[( ( GC_IO_AI_COUNT * 2 ) + 2 ) + i] );

		cal_vals_l1[i] = cv_l1.get_value();
		cal_vals_l2[i] = cv_l2.get_value();
	}

	BBB_HVAC::IOCOMM::PMIC_CACHE_ENTRY pmic_value( parts[GC_IO_AI_COUNT + 1] );

	pmic_ai_en =  pmic_value.get_value( ) & GC_PMIC_AI_EN_MASK;
	pmic_ai_fault =  pmic_value.get_value( ) & GC_PMIC_AI_ERR_MASK;
	pmic_do_en = pmic_value.get_value( ) & GC_PMIC_DO_EN_MASK;
	pmic_do_fault = pmic_value.get_value( ) & GC_PMIC_DO_ERR_MASK;

	BBB_HVAC::IOCOMM::DO_CACHE_ENTRY do_value( parts[GC_IO_AI_COUNT] );
	uint8_t mask = 1;

	for ( size_t i = 0; i < ( size_t )do_states.size(); i++ )
	{
		do_states[i]  = do_value.get_value( ) & mask;
		//LOG_DEBUG_STAT( "DO:" + num_to_str( i ) + " -> " + num_to_str( do_states[i] ) );
		mask = mask << 1;
	}

	this->sig_get_status( _message.board_id, adc_values, do_states, pmic_do_en, pmic_do_fault, pmic_ai_en, pmic_ai_fault, cal_vals_l1, cal_vals_l2 );

	return;
}
void MESSAGE_BUS::emit_point_label_message( COMMANDS _command, const BBB_HVAC::MESSAGE_PTR& _data )
{
	QVector<QVector<QString>> data( _data->get_part_count() );

	for ( size_t i = 0; i < _data->get_part_count( ); i++ )
	{
		BBB_HVAC::BOARD_POINT board_point = BBB_HVAC::BOARD_POINT::from_string( _data->get_part_as_s( i ) );
		QVector<QString> temp_vect( 3 );
		temp_vect[0] = QString( board_point.get_board_tag( ).data( ) );
		temp_vect[1] = QString::number( board_point.get_point_id( ) );
		temp_vect[2] = QString( board_point.get_description( ).data( ) );
		data[i] = temp_vect;
	}

	sig_point_label_data( _command, data );
	return;
}

void MESSAGE_BUS::emit_map_data_mesage( COMMANDS _command, const BBB_HVAC::MESSAGE_PTR& _data )
{
	std::map<std::string, std::string> map;
	BBB_HVAC::MESSAGE::message_to_map( _data, map );
	QMap<QString, QVector<QString>> data;

	for ( auto map_iterator = map.cbegin( ); map_iterator != map.cend( ); ++map_iterator )
	{
		//LOG_DEBUG( map_iterator->second );
		BBB_HVAC::BOARD_POINT board_point = BBB_HVAC::BOARD_POINT::from_string( map_iterator->second );
		QVector<QString> temp_vect( 4 );
		temp_vect[0] = QString( board_point.get_board_tag( ).data( ) ) ;
		temp_vect[1] = QString( BBB_HVAC::CONFIG_ENTRY::type_to_string( board_point.get_type( ) ).data( ) );
		temp_vect[2] = QString::number( board_point.get_point_id( ) );
		temp_vect[3] = QString( board_point.get_description( ).data( ) );
		data.insert( QString( map_iterator->first.data( ) ) , temp_vect );
	}

	sig_point_map_data( _command, data );
	return;
}

void MESSAGE_BUS::emit_set_point_data_message( COMMANDS _command, const  BBB_HVAC::MESSAGE_PTR& _data )
{
	QMap<QString, QString> data;

	for ( unsigned int row_idx = 0; row_idx < _data->get_part_count( ); ++row_idx )
	{
		std::string message_part = _data->get_part_as_s( row_idx );
		BBB_HVAC::SET_POINT sp = BBB_HVAC::SET_POINT::from_string( message_part );
		data.insert( QString( sp.get_description( ).data( ) ), QString::number( sp.get_value( ) ) );
	}

	sig_set_point_data( _command, data );
	return;
}

void MESSAGE_BUS::emit_logic_status_update_message( COMMANDS _command, const BBB_HVAC::MESSAGE_PTR& _data )
{
	QMap<QString, QString> data;
	std::map<std::string, std::string> map;
	BBB_HVAC::MESSAGE::message_to_map( _data, map );

	for ( auto map_iterator = map.begin( ); map_iterator != map.end( ); ++map_iterator )
	{
		data.insert( QString( map_iterator->first.data( ) ), QString( map_iterator->second.data( ) ) );
	}

	sig_logic_status_data( _command, data );
	return;
}

bool MESSAGE_BUS::is_connected( void ) const
{
	if ( this->ctx == nullptr )
	{
		return false;
	}
	else
	{
		return true;
	}
}