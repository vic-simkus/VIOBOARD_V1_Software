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

#include "BOARD_INFO_WIDGET.h"
#include "RAW_BOARD_INFO.h"
#include "LOGIC_INFO.h"
#include "DEBUG_FRAME.h"
#include <QDateTime>

DEF_LOGGER_STAT( "QT_HMI" );

MAIN_WINDOW::MAIN_WINDOW( )
{
	this->main_widget = new QTabWidget( this );
	this->setCentralWidget( main_widget );
	QWidget* logic_widget = nullptr;
	this->main_widget->addTab( ( logic_widget = new LOGIC_INFO( nullptr ) ), "LOGIC INFO" );
	this->main_widget->addTab( new RAW_BOARD_INFO( nullptr ), "RAW BOARD INFO" );
	this->main_widget->addTab( new DEBUG_FRAME( nullptr ), "DEBUG" );
	this->statusBar()->setSizeGripEnabled( true );
	this->statusBar()->showMessage( "Application started." );
	connect( logic_widget, SIGNAL( sig_update_start() ), this, SLOT( slot_update_start() ) );
	connect( logic_widget, SIGNAL( sig_update_finish() ), this, SLOT( slot_update_finish() ) );
	return;
}

void MAIN_WINDOW::slot_update_start( void )
{
	this->statusBar()->showMessage( "Update started." );
	return;
}
void MAIN_WINDOW::slot_update_finish( void )
{
	this->statusBar()->showMessage( "Update finished @ " + QDateTime::currentDateTime().toString( Qt::ISODate ) );
	return;
}

