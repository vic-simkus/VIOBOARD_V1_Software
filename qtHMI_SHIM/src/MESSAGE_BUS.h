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
#ifndef MESSAGE_BUS_H
#define MESSAGE_BUS_H

#include <QObject>

class MESSAGE_BUS : public QObject
{
		Q_OBJECT;
	public:
		MESSAGE_BUS();
	public slots:
		void slot_raw_adc_value_changed( const QString& _board, uint8_t _io, uint16_t _value );
	signals:
		void sig_raw_adc_value_changed( const QString& _board, uint8_t _io, uint16_t _value );
};

#endif