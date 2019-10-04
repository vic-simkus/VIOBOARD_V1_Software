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

#ifndef PMIC_INDICATOR_H
#define PMIC_INDICATOR_H

#include <QFrame>

class QPushButton;

class PMIC_INDICATOR : public QFrame
{
		Q_OBJECT;
	public:
		PMIC_INDICATOR( bool _enabled, bool _faulted );

		void set_enabled( void );
		void set_disabled( void );
		void set_faulted( void );
		void set_notfaulted( void );

		bool get_is_faulted( void ) const;
		bool get_is_enabled( void ) const;

	signals:
		void reset_clicked( void );
		void enable_clicked( void );

	public slots:
		void update_status( void );

	protected slots:
		void __button_clicked( void );

	protected:
		QPushButton* cmd_button;

		bool is_faulted;
		bool is_enabled;
} ;


#endif /* PMIC_INDICATOR_H */

