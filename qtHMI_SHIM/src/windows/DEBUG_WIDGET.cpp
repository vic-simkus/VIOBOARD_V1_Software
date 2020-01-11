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

#include "DEBUG_WIDGET.h"

#include "widgets/DEBUG_FORCE_WIDGET.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>

#include "globals.h"

DEF_LOGGER_STAT( "DEBUG_WIDGET" );

DEBUG_WIDGET::DEBUG_WIDGET( const QString& _board_id ) : QFrame( )
{
	this->board_tag = _board_id;
	//this->setLayout( new QVBoxLayout() );
	QHBoxLayout* h_layout = new QHBoxLayout();
	QVBoxLayout* v_layout = new QVBoxLayout();
	//this->layout()->addItem( h_layout );
	h_layout->setContentsMargins( 4, 4, 4, 4 );
	h_layout->setSpacing( 2 );

	for ( size_t i = 0; i < GC_IO_AI_COUNT; i++ )
	{
		this->force_widget[i] = new DEBUG_FORCE_WIDGET( "AI" + QString::number( i ), i );
		h_layout->addWidget( this->force_widget[i] );
		connect( this->force_widget[i], SIGNAL( sig_force_clicked( unsigned int, bool , uint16_t ) ), this, SLOT( slot_force_clicked( unsigned int, bool , uint16_t ) ) );
	}

	v_layout->addItem( h_layout );
	v_layout->addItem( new QSpacerItem( 1, 1, QSizePolicy::Maximum, QSizePolicy::MinimumExpanding ) );
	this->setLayout( v_layout );

	connect( message_bus, SIGNAL( sig_raw_adc_value_changed( const QString&, uint8_t , uint16_t ) ) , this, SLOT( slot_raw_adc_value_changed( const QString&, uint8_t , uint16_t ) ) ) ;
}

void DEBUG_WIDGET::slot_raw_adc_value_changed( const QString& _board, uint8_t _io, uint16_t _value )
{
	if ( _board != this->board_tag )
	{
		return;
	}

	this->force_widget[_io]->slot_update_real_value( _value );
	return;
}

void DEBUG_WIDGET::slot_force_clicked( unsigned int _port, bool _state, uint16_t _value )
{
	LOG_DEBUG( "FORCE: " + this->board_tag.toStdString() +  ": port: " + num_to_str( _port ) + ", forced: " + ( _state ? "yes" : "no" ) + ", value: " + num_to_str( _value ) );

	if ( _state )
	{
		message_bus->add_message( MESSAGE_BUS::MESSAGE::create_message_force_ai_value( this->board_tag, _port, _value ) );
		sig_ai_forced( this->board_tag, _port, _value );
	}
	else
	{
		message_bus->add_message( MESSAGE_BUS::MESSAGE::create_message_unforce_ai_value( this->board_tag, _port ) );
		sig_ai_unforced( this->board_tag,  _port );
	}

	return;
}
