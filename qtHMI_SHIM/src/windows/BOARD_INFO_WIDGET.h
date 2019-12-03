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

#ifndef BOARD_INFO_WIDGET_H
#define BOARD_INFO_WIDGET_H

#include "ui_components.h"
#include "globals.h"

#include <QFrame>

#define DATA_UPDATE_TIMER 500

class QGridLayout;
class QLabel;
class QPushButton;

class QSignalMapper;

class BOARD_INFO_WIDGET : public QFrame
{
		Q_OBJECT;
	public:
		BOARD_INFO_WIDGET( const QString& _board_id );
	protected:
		QGridLayout* do_grid_layout;
		QGridLayout* ai_grid_layout;
		QGridLayout* pmic_grid_layout;

		void setup_do_stuff( void );
		void setup_ai_stuff( void );
		void setup_pmic_stuff( void );

		uint8_t get_pmic_flags( void ) const;

	private:
		AI_VALUE* ai_values[GC_IO_AI_COUNT];
		AI_RAW_VALUE* ai_raw_values[GC_IO_AI_COUNT];
		DO_INDICATOR* do_values[GC_IO_DO_COUNT];
		CAL_VALUE* cal_l1_values[GC_IO_AI_COUNT];
		CAL_VALUE* cal_l2_values[GC_IO_AI_COUNT];

		PMIC_INDICATOR* ai_pmic;
		PMIC_INDICATOR* do_pmic;

		QTimer* timer;

		QSignalMapper* do_button_mapper;

		QString board_id;

		bool update_l1_cal_values;
		bool update_l2_cal_values;

		void update_cal_ui_values( CAL_VALUE** _cal_ui, const QVector<uint16_t>& _val );

		void send_cal_values();

	private slots:
		void update_data( void );

		void slot_get_status( const QString& _board, const QVector<uint16_t>& _adc_values, const QVector<bool>& _do_states, bool _pmic_do_en, bool _pmic_do_fault, bool _pmic_ai_en, bool _pmic_ai_fault, const QVector<uint16_t>& _cal_vals_l1, const QVector<uint16_t>& _cal_vals_l2 );

		void cmd_enable_do_pmic_clicked( void );
		void cmd_enable_ai_pmic_clicked( void );

		void cmd_reset_do_pmic_clicked( void );
		void cmd_reset_ai_pmic_clicked( void );

		void cmd_enable_do_clicked( int _do );

		void manage_pmic_status( uint8_t _mask );

		void update_l1_cal_values_clicked( void );
		void update_l2_cal_values_clicked( void );

		void send_l1_cal_values_clicked( void );
		void send_l2_cal_values_clicked( void );
};



#endif /* BOARD_INFO_WIDGET_H */

