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

#include "MAIN_WINDOW.h"

#include "lib/bbb_hvac.hpp"
#include "globals.h"

#include "../widgets/MAIN_WIDGET.h"


#include <QStatusBar>
#include <QDateTime>

DEF_LOGGER_STAT( "QT_HMI" );

MAIN_WINDOW::MAIN_WINDOW( const QStringList& _board_list )
{
	this->main_widget = new MAIN_WIDGET( this, _board_list );
	this->setCentralWidget( main_widget );

	this->statusBar()->showMessage( "Application started." );

	connect( message_bus, SIGNAL( sig_update_started() ), this, SLOT( slot_update_start() ) );
	connect( message_bus, SIGNAL( sig_update_finished() ), this, SLOT( slot_update_finish() ) );

	return;
}

void MAIN_WINDOW::slot_update_start( void )
{
	//this->statusBar()->showMessage( "Update started." );
	return;
}
void MAIN_WINDOW::slot_update_finish( void )
{
	this->statusBar()->showMessage( "Update finished @ " + QDateTime::currentDateTime().toString( Qt::ISODate ) );
	return;
}

