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

#include "PMIC_INDICATOR.h"
#include <QVBoxLayout>
#include <QPushButton>

PMIC_INDICATOR::PMIC_INDICATOR(bool _enabled,bool _faulted ) : QFrame( )
{
	this->is_enabled = _enabled;
	this->is_faulted = _faulted;
	
	QVBoxLayout * layout = new QVBoxLayout( this );
	this->setLayout( layout );
	
	this->cmd_button = new QPushButton( "...", this );

	layout->addWidget( this->cmd_button );
	this->update_status();
	
	connect(this->cmd_button,SIGNAL(clicked()),this,SLOT(__button_clicked()));
}

void PMIC_INDICATOR::__button_clicked(void)
{
	if(this->is_faulted)
	{
		emit(this->reset_clicked());
	}
	else
	{
		emit(this->enable_clicked());
	}
}
bool PMIC_INDICATOR::get_is_faulted(void) const
{
	return this->is_faulted;
}
bool PMIC_INDICATOR::get_is_enabled(void) const
{
	return this->is_enabled;
}

void PMIC_INDICATOR::set_enabled( void )
{
	this->is_enabled = true;
	this->update_status();		
}

void PMIC_INDICATOR::set_disabled( void )
{
	this->is_enabled = false;
	this->update_status();
}

void PMIC_INDICATOR::set_faulted( void )
{
	this->is_faulted = true;
	this->update_status();
}

void PMIC_INDICATOR::set_notfaulted( void )
{
	this->is_faulted = false;
	this->update_status();
}
void PMIC_INDICATOR::update_status(void)
{
	if(this->is_faulted)
	{
		this->cmd_button->setStyleSheet( "background: red" );
		this->cmd_button->setText( "Click to reset" );
	}
	else
	{
		if(this->is_enabled)
		{
			this->cmd_button->setStyleSheet( "background: green" );
			this->cmd_button->setText( "Click to disable" );
		}
		else
		{
			this->cmd_button->setStyleSheet( "background: gray" );
			this->cmd_button->setText( "Click to enable" );		
		}
	}
}