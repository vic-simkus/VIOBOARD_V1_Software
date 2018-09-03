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

#include "DO_INDICATOR.h"
#include <QVBoxLayout>

DO_INDICATOR::DO_INDICATOR( ) : QPushButton( )
{
	QVBoxLayout * layout = new QVBoxLayout( this );
	this->setLayout( layout );
	this->setText( "..." );

	this->set_disabled( );
}

void DO_INDICATOR::set_enabled( void )
{
	this->setStyleSheet( "background: green" );
	this->setText( "Click to Disable" );
	this->is_enabled = true;
}

void DO_INDICATOR::set_disabled( void )
{
	this->setStyleSheet( "background: gray" );
	this->setText( "Click to Enable" );
	this->is_enabled = false;
}

bool DO_INDICATOR::get_enabled( void ) const
{
	return this->is_enabled;
}
