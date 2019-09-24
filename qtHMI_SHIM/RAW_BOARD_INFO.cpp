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

#include "RAW_BOARD_INFO.h"
#include "BOARD_INFO_WIDGET.h"
#include <QVBoxLayout>

RAW_BOARD_INFO::RAW_BOARD_INFO( QWidget* _p ) : QFrame( _p )
{
	this->main_widget = new QTabWidget( this );
	this->setLayout( new QVBoxLayout( this ) );
	this->layout()->addWidget( this->main_widget );
	this->main_widget->addTab( new BOARD_INFO_WIDGET( "BOARD1" ), "BOARD1" );
	this->main_widget->addTab( new BOARD_INFO_WIDGET( "BOARD2" ), "BOARD2" );
}


RAW_BOARD_INFO::~RAW_BOARD_INFO( )
{
}

