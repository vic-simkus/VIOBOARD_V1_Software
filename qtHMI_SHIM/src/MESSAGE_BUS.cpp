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


DEF_LOGGER_STAT( "MESSAGE_BUS" );
MESSAGE_BUS::MESSAGE_BUS( uint8_t _update_frequency ) : QObject()
{
	// The timer that will periodically invoke the message processing logic
	this->timer_update = new QTimer( this );
	this->update_counter = 0;
	this->update_frequency = _update_frequency;
	// On timer fire run the message processing logic
	connect( this->timer_update, SIGNAL( timeout() ), this, SLOT( do_update() ) );
	this->ctx = nullptr;
	this->failure_count = 0;
	this->timer_update->start( 1000 / update_frequency );
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
		LOG_DEBUG_STAT( "Disconnection attempt failed." );
	}

	this->ctx = nullptr;
	return;
}

void MESSAGE_BUS::do_update( void )
{
	/*
	Invoked by the timer this->update_frequency times per second.
	*/

	/*
	Check if we're connected
	*/
	if ( this->ctx == nullptr )
	{

		// If not, connect.
		this->ctx = BBB_HVAC::CLIENT::CLIENT_CONTEXT::create_instance( );

		try
		{
			this->ctx->connect( );
		}
		catch ( const BBB_HVAC::EXCEPTIONS::CONNECTION_ERROR& _e )
		{
			LOG_ERROR_STAT( "Failed to connect: " +  std::string( _e.what() ) );
			delete this->ctx;
			this->ctx = nullptr;
			this->failure_count += 1;
		}

		LOG_DEBUG_STAT( "MESSAGE_BUS connected to remote point." );
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
		DEF_LOGGER_STAT( "Processing message: " + num_to_str( ( int )c.first ) );

		switch ( c.first )
		{
			case COMMANDS::NONE:
				// Do nothing;
				break;

			case COMMANDS::GET_LABELS_DO:
				message = this->ctx->message_processor->create_get_labels_message_request( BBB_HVAC::ENUM_CONFIG_TYPES::DO );
				message = this->ctx->send_message_and_wait( message );
				this->emit_point_label_message( c.first, message );
				break;

			case COMMANDS::GET_LABELS_AI:
				message = this->ctx->message_processor->create_get_labels_message_request( BBB_HVAC::ENUM_CONFIG_TYPES::AI );
				message = this->ctx->send_message_and_wait( message );
				this->emit_point_label_message( c.first, message );
				break;

			case COMMANDS::GET_LABELS_MAP:
				message = this->ctx->message_processor->create_get_labels_message_request( BBB_HVAC::ENUM_CONFIG_TYPES::MAP );
				message = this->ctx->send_message_and_wait( message );
				this->emit_map_data_mesage( c.first, message );
				break;

			case COMMANDS::GET_SET_POINTS:
				message = this->ctx->message_processor->create_get_labels_message_request( BBB_HVAC::ENUM_CONFIG_TYPES::SP );
				message = this->ctx->send_message_and_wait( message );
				this->emit_set_point_data_message( c.first, message );
				break;

			case COMMANDS::GET_LOGIC_STATUS:
				message = this->ctx->message_processor->create_read_logic_status( );
				message = this->ctx->send_message_and_wait( message );
				this->emit_logic_status_update_message( c.first, message );
				break;
		};
	}
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
		LOG_DEBUG_STAT( map_iterator->second );
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