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

#include "CAL_VALUE.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QPushButton>

CAL_VALUE::CAL_VALUE()  : QFrame(nullptr)
{
	QVBoxLayout * main_layout = new QVBoxLayout(this);
	this->setLayout(main_layout);
	
	this->setFrameShape(Shape::Box);
	
	this->value = new QSpinBox(this);	
	this->value->setRange(-255,255);
	
	main_layout->addWidget(this->value);
	
	this->cmd_submit = new QPushButton("Submit",this);
	
	main_layout->addWidget(cmd_submit);
	
	connect(this->cmd_submit,SIGNAL(clicked()),this,SLOT(__submit_clicked()));
	connect(this->value,SIGNAL(valueChanged(int)),this,SLOT(__value_changed()));
}
int CAL_VALUE::get_value(void)
{
	return this->value->value();
}
void CAL_VALUE::__value_changed(void)
{
	this->cmd_submit->setStyleSheet("background-color: yellow");
}
void CAL_VALUE::__submit_clicked(void)
{
	this->reset_status();
	this->clicked();
}

void CAL_VALUE::reset_status(void)
{
	this->cmd_submit->setStyleSheet("");	
}

void CAL_VALUE::set_value(int _val)
{
	this->value->setValue(_val);
	return;
}

