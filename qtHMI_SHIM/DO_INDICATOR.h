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

#ifndef DO_INDICATOR_H
#define DO_INDICATOR_H

#include <QPushButton>

class DO_INDICATOR : public QPushButton
{
	Q_OBJECT;
public:
	DO_INDICATOR( );

	void set_enabled( void );
	void set_disabled( void );

	bool get_enabled( void ) const;
protected:
	bool is_enabled;
};

#endif /* DO_INDICATOR_H */

