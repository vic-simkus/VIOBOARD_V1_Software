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

#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H

#include <QMainWindow>

class MAIN_WIDGET;

class MAIN_WINDOW : public QMainWindow
{
		Q_OBJECT;
	public:
		MAIN_WINDOW( );

	protected:
		MAIN_WIDGET* main_widget;
		QWidget* update_beacon;

	protected slots:
		void slot_update_start( void );
		void slot_update_finish( void );


};

#endif /* _MAIN_WINDOW_H */
