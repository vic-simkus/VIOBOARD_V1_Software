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

#include "DEBUG_FRAME.h"
#include "DEBUG_WIDGET.h"

#include <QTabWidget>
#include <QVBoxLayout>

DEBUG_FRAME::DEBUG_FRAME( QWidget* _p ) : QFrame( _p )
{
	this->main_widget = new QTabWidget( this );
	this->setLayout( new QVBoxLayout( this ) );
	this->layout()->addWidget( this->main_widget );
	this->main_widget->addTab( new DEBUG_WIDGET( "BOARD1" ), "BOARD1" );
	this->main_widget->addTab( new DEBUG_WIDGET( "BOARD2" ), "BOARD2" );
}


DEBUG_FRAME::~DEBUG_FRAME( )
{
}

