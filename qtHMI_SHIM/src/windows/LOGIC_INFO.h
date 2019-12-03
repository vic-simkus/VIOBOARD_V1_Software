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

#ifndef LOGIC_INFO_H
#define LOGIC_INFO_H

#include <QFrame>

#include "lib/bbb_hvac.hpp"

#include "MESSAGE_BUS.h"

class QGroupBox;
class QSplitter;
class QTableWidget;

class LOGIC_INFO : public QFrame
{
		Q_OBJECT;
	public:
		LOGIC_INFO( QWidget* _p );
		virtual ~LOGIC_INFO( );

	protected:

		void update_status( void );
	private:

		QGroupBox* group_logic_points;
		QGroupBox* group_io_points;
		QGroupBox* group_point_map;

		QSplitter* splitter_io_points;
		QSplitter* splitter_main_window;
		QSplitter* splitter_logic_points;

		QTableWidget* table_sp_points;
		QTableWidget* table_ai_points;
		QTableWidget* table_do_points;
		QTableWidget* table_map_points;
		QTableWidget* table_point_values;

		void dump_message_parts( BBB_HVAC::MESSAGE_PTR& _message );

	private slots:
		void slot_mb_label_data( MESSAGE_BUS::COMMANDS, const QVector<QVector<QString>>& _data );
		void slot_mb_map_data( MESSAGE_BUS::COMMANDS _cmd, const QMap<QString, QVector<QString>>& _data );
		void slot_mb_logic_status_update( MESSAGE_BUS::COMMANDS _cmd, const QMap<QString, QString>& _data );
		void slot_mb_set_point_update( MESSAGE_BUS::COMMANDS _cmd, const QMap<QString, QString>& _data );
} ;

#endif /* LOGIC_INFO_H */

