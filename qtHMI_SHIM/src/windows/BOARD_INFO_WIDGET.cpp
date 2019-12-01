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

#include "BOARD_INFO_WIDGET.h"

#include <QSignalMapper>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QMessageBox>

#include "globals.h"

DEF_LOGGER_STAT( "BOARD_INFO_WIDGET" );

void BOARD_INFO_WIDGET::setup_do_stuff( void )
{
	this->do_grid_layout->addWidget( new QLabel( "<i><center>DO01" ), 0, 0 );
	this->do_grid_layout->addWidget( new QLabel( "<i><center>DO02" ), 0, 1 );
	this->do_grid_layout->addWidget( new QLabel( "<i><center>DO03" ), 0, 2 );
	this->do_grid_layout->addWidget( new QLabel( "<i><center>DO04" ), 0, 3 );

	for ( size_t i = 0; i < DO_COUNT; i++ )
	{
		this->do_values[i] = new DO_INDICATOR( );
		this->do_grid_layout->addWidget( this->do_values[i], 1, i );
		this->do_button_mapper->setMapping( this->do_values[i], i );
		connect( this->do_values[i], SIGNAL( clicked( ) ), this->do_button_mapper, SLOT( map( ) ) );
	}
}

void BOARD_INFO_WIDGET::setup_ai_stuff( void )
{
	this->ai_grid_layout->addWidget( new QLabel( "<i><center>AI01<br/>(4-20mA)</center></i>" ), 0, 0 );
	this->ai_grid_layout->addWidget( new QLabel( "<i><center>AI02<br/>(4-20mA)</center></i>" ), 0, 1 );
	this->ai_grid_layout->addWidget( new QLabel( "<i><center>AI03<br/>(4-20mA)</center></i>" ), 0, 2 );
	this->ai_grid_layout->addWidget( new QLabel( "<i><center>AI04<br/>(4-20mA)</center></i>" ), 0, 3 );
	this->ai_grid_layout->addWidget( new QLabel( "<i><center>AI05<br/>(ICTD)</center></i>" ), 0, 4 );
	this->ai_grid_layout->addWidget( new QLabel( "<i><center>AI06<br/>(ICTD)</center></i>" ), 0, 5 );
	this->ai_grid_layout->addWidget( new QLabel( "<i><center>AI07<br/>(ICTD)</center></i>" ), 0, 6 );
	this->ai_grid_layout->addWidget( new QLabel( "<i><center>AI08<br/>(ICTD)</center></i>" ), 0, 7 );

	for ( size_t i = 0; i < AI_COUNT; i++ )
	{
		this->ai_grid_layout->addWidget( new QLabel( "<i><center>Volt Value</center></i>" ), 1, i );
		this->ai_values[i] = new AI_VALUE( );
		this->ai_grid_layout->addWidget( this->ai_values[i], 2, i );
	}

	for ( size_t i = 0; i < AI_COUNT; i++ )
	{
		this->ai_grid_layout->addWidget( new QLabel( "<i><center>Raw Value</center></i>" ), 3, i );
		this->ai_raw_values[i] = new AI_RAW_VALUE( );
		this->ai_grid_layout->addWidget( this->ai_raw_values[i], 4, i );
	}

	for ( size_t i = 0; i < AI_COUNT; i++ )
	{
		this->ai_grid_layout->addWidget( new QLabel( "<i><center>L1 Calibration</center></i>" ), 5, i );
		this->cal_l1_values[i] = new CAL_VALUE( );
		this->ai_grid_layout->addWidget( this->cal_l1_values[i], 6, i );
		connect( this->cal_l1_values[i], SIGNAL( clicked() ), this, SLOT( send_l1_cal_values_clicked() ) );
	}

	QPushButton* tmp_cmd = new QPushButton( "Refresh L1 Calibration Values" );
	connect( tmp_cmd, SIGNAL( clicked() ), this, SLOT( update_l1_cal_values_clicked() ) );
	this->ai_grid_layout->addWidget( tmp_cmd, 7, 0, 1, 8 );

	for ( size_t i = 0; i < AI_COUNT; i++ )
	{
		this->ai_grid_layout->addWidget( new QLabel( "<i><center>L2 Calibration</center></i>" ), 8, i );
		this->cal_l2_values[i] = new CAL_VALUE( );
		this->ai_grid_layout->addWidget( this->cal_l2_values[i], 9, i );
		connect( this->cal_l2_values[i], SIGNAL( clicked() ), this, SLOT( send_l2_cal_values_clicked() ) );
	}

	tmp_cmd = new QPushButton( "Refresh L2 Calibration Values" );
	connect( tmp_cmd, SIGNAL( clicked() ), this, SLOT( update_l2_cal_values_clicked() ) );
	this->ai_grid_layout->addWidget( tmp_cmd, 10, 0, 1, 8 );

	connect( message_bus, SIGNAL( sig_get_status( const QString&, const QVector<uint16_t>&, const QVector<bool>&, bool , bool , bool, bool ) ), this, SLOT( slot_get_status( const QString&, const QVector<uint16_t>&, const QVector<bool>&, bool , bool , bool , bool ) ) );
}

void BOARD_INFO_WIDGET::send_l1_cal_values_clicked( void )
{
	LOG_DEBUG_STAT( "Sending L1 cal values." );
	send_cal_values( this->cal_l1_values, 1 );
}

void BOARD_INFO_WIDGET::send_l2_cal_values_clicked( void )
{
	LOG_DEBUG_STAT( "Sending L2 cal values." );
	send_cal_values( this->cal_l2_values, 2 );
}

void BOARD_INFO_WIDGET::send_cal_values( CAL_VALUE** _cal_values, unsigned char _level )
{
	uint16_t cal_val = 0;
	CAL_VALUE_ARRAY calibration_values;

	for ( size_t i = 0; i < AI_COUNT; i++ )
	{
		int val = _cal_values[i]->get_value();

		if ( val == 0 )
		{
			cal_val = 0;
		}
		else if ( val < 0 )
		{
			/*
			 * Less than zero.  Calibration is "down".
			 */
			cal_val = ( ( unsigned int )( -val ) ) & 0x00FF;
		}
		else
		{
			/*
			 * Greater than zero.  Calibration is "up".
			 */
			cal_val = ( ( ( unsigned int )val ) << 8 ) & 0xFF00;
		}

		LOG_DEBUG_STAT( "Value [" + num_to_str( i ) + "] -> [" + num_to_str( val ) + "] -> [" + num_to_str( cal_val ) + "]" );
		calibration_values.push_back( cal_val );
	}

	MESSAGE_PTR m;

	if ( _level == 1 )
	{
		m = this->ctx->message_processor->create_set_l1_cal_vals( this->board_id.toStdString(), calibration_values );
	}
	else
	{
		m = this->ctx->message_processor->create_set_l2_cal_vals( this->board_id.toStdString(), calibration_values );
	}

	LOG_DEBUG_STAT( m->to_string() );
	ctx->send_message( m );
}

void BOARD_INFO_WIDGET::update_l1_cal_values_clicked( void )
{
	this->update_l1_cal_values = true;
}
void BOARD_INFO_WIDGET::update_l2_cal_values_clicked( void )
{
	this->update_l2_cal_values = true;
}

void BOARD_INFO_WIDGET::setup_pmic_stuff( void )
{
	this->do_pmic = new PMIC_INDICATOR( false, false );
	this->ai_pmic = new PMIC_INDICATOR( false, false );
	connect( this->do_pmic, SIGNAL( enable_clicked( ) ), this, SLOT( cmd_enable_do_pmic_clicked( ) ) );
	connect( this->do_pmic, SIGNAL( reset_clicked( ) ), this, SLOT( cmd_reset_do_pmic_clicked( ) ) );
	connect( this->ai_pmic, SIGNAL( enable_clicked( ) ), this, SLOT( cmd_enable_ai_pmic_clicked( ) ) );
	connect( this->ai_pmic, SIGNAL( reset_clicked( ) ), this, SLOT( cmd_reset_ai_pmic_clicked( ) ) );
	this->pmic_grid_layout->addWidget( new QLabel( "<i>DO PMIC" ), 0, 0 );
	this->pmic_grid_layout->addWidget( new QLabel( "<i>AI PMIC" ), 0, 1 );
	this->pmic_grid_layout->addWidget( this->do_pmic, 1, 0 );
	this->pmic_grid_layout->addWidget( this->ai_pmic, 1, 1 );
	return;
}

BOARD_INFO_WIDGET::BOARD_INFO_WIDGET( const QString& _board_id ) : QFrame( )
{
	this->board_id = _board_id;
	QVBoxLayout* main_layout = new QVBoxLayout( );
	this->setLayout( main_layout );
	main_layout->addWidget( new QLabel( "<b>" + this->board_id ) );
	main_layout->addWidget( new QLabel( "<b>Digital Outputs" ) );
	this->do_grid_layout = new QGridLayout( );
	main_layout->addLayout( do_grid_layout );
	main_layout->addWidget( new QLabel( "<b>Analog Inputs" ) );
	this->ai_grid_layout = new QGridLayout( );
	main_layout->addLayout( ai_grid_layout );
	main_layout->addWidget( new QLabel( "<b>PMIC Status" ) );
	this->pmic_grid_layout = new QGridLayout( );
	main_layout->addLayout( pmic_grid_layout );
	this->do_button_mapper = new QSignalMapper( this );
	connect( this->do_button_mapper, SIGNAL( mapped( int ) ), this, SLOT( cmd_enable_do_clicked( int ) ) );
	this->setup_do_stuff( );
	this->setup_ai_stuff( );
	this->setup_pmic_stuff( );
	ctx = nullptr;
	this->timer = new QTimer( this );
	connect( this->timer, SIGNAL( timeout( ) ), this, SLOT( update_data( ) ) );
	this->timer->start( DATA_UPDATE_TIMER );
	return;
}

void BOARD_INFO_WIDGET::slot_get_status( const QString& _board, const QVector<uint16_t>& _adc_values, const QVector<bool>& _do_states, bool _pmic_do_en, bool _pmic_do_fault, bool _pmic_ai_en, bool _pmic_ai_fault )
{
	if ( _board != this->board_id )
	{
		return;
	}

	//LOG_DEBUG_STAT( "Updating status for: " + _board.toStdString() );

	for ( size_t i = 0; i < ( size_t )_adc_values.size(); ++i )
	{
		this->ai_raw_values[i]->set_value( _adc_values[i] );
		this->ai_values[i]->set_value( AI_ADC_STEP * ( float ) _adc_values[i] );

		message_bus->slot_raw_adc_value_changed( this->board_id, i, _adc_values[i] );
	}

	uint8_t mask = 1;

	for ( size_t i = 0; i < ( size_t )_do_states.size(); i++ )
	{
		if ( _do_states[i] )
		{
			this->do_values[i]->set_enabled( );
		}
		else
		{
			this->do_values[i]->set_disabled( );
		}

		mask = mask << 1;
	}

	if ( _pmic_ai_en )
	{
		this->ai_pmic->set_enabled( );
	}
	else
	{
		this->ai_pmic->set_disabled( );
	}

	if ( _pmic_ai_fault )
	{
		this->ai_pmic->set_faulted( );
	}
	else
	{
		this->ai_pmic->set_notfaulted( );
	}

	if ( _pmic_do_en )
	{
		this->do_pmic->set_enabled( );
	}
	else
	{
		this->do_pmic->set_disabled( );
	}

	if ( _pmic_do_fault )
	{
		this->do_pmic->set_faulted( );
	}
	else
	{
		this->do_pmic->set_notfaulted( );
	}
}
/*
MESSAGE_PTR BOARD_INFO_WIDGET::update_data_and_return( void )
{


		if ( first_run )
		{
			this->update_cal_ui_values( parts );
		}
		else
		{
			if ( this->update_l1_cal_values )
			{
				this->update_l1_cal_values = false;
				this->update_cal_l1_ui_values( parts );
			}

			if ( this->update_l2_cal_values )
			{
				this->update_l2_cal_values = false;
				this->update_cal_l2_ui_values( parts );
			}
		}

	return m;
}*/
void BOARD_INFO_WIDGET::update_cal_l1_ui_values( const vector<string>& parts )
{
	size_t offset = AI_COUNT + 2;	// Skip past the ADC values and the DO and PMIC statuses
	size_t j = 0;

	for ( size_t i = offset; i < offset + AI_COUNT; ++i )
	{
		IOCOMM::CAL_VALUE_ENTRY cv( parts[i] );
		uint16_t value = cv.get_value();
		int add_val = ( ( value >> 8 ) & 0x00FF );
		int sub_val = -( value & 0x00FF );
		int cal_val = add_val + sub_val;
		this->cal_l1_values[j]->set_value( cal_val );
		this->cal_l1_values[j]->reset_status();
		j += 1;
	}

	return;
}
void BOARD_INFO_WIDGET::update_cal_l2_ui_values( const vector<string>& parts )
{
	size_t offset = ( AI_COUNT * 2 ) + 2;			// Skip past the L1 cache values.
	size_t j = 0;

	for ( size_t i = offset; i < parts.size(); ++i )
	{
		IOCOMM::CAL_VALUE_ENTRY cv( parts[i] );
		uint16_t value = cv.get_value();
		int add_val = ( ( value >> 8 ) & 0x00FF );
		int sub_val = -( value & 0x00FF );
		int cal_val = add_val + sub_val;
		this->cal_l2_values[j]->set_value( cal_val );
		this->cal_l2_values[j]->reset_status();
		j += 1;

		if ( j == AI_COUNT )
		{
			break;
		}
	}

	return;
}
void BOARD_INFO_WIDGET::update_cal_ui_values( const vector<string>& parts )
{
	this->update_cal_l1_ui_values( parts );
	this->update_cal_l2_ui_values( parts );
	return;
}

void BOARD_INFO_WIDGET::update_data( void )
{
	message_bus->add_message( MESSAGE_BUS::MESSAGE( this->board_id , MESSAGE_BUS::COMMANDS::GET_STATUS ) );
	return;
}

void BOARD_INFO_WIDGET::manage_pmic_status( uint8_t _mask )
{
	this->timer->stop( );
	LOG_DEBUG_STAT( "Manage PMIC status.  Mask: " + num_to_str( _mask ) );
	MESSAGE_PTR m; // = this->update_data_and_return( );
	const vector<string>& parts = m->get_parts( );
	IOCOMM::PMIC_CACHE_ENTRY ai_value( parts[AI_COUNT + 1] );
	uint8_t status = ai_value.get_value( );

	if ( _mask == PMIC_AI_ERR_MASK || _mask == PMIC_DO_ERR_MASK )
	{
		/*
		 * We're in reset mode
		 */
		/*
		 * We technically don't have to do anything here.  Writing back the same status will attempt a reset since the fault flag will be '1'
		 */
	}
	else
	{
		/*
		 * We're in the enable/disable mode
		 */
		if ( status & _mask )
		{
			/*
			 * Currently enabled.
			 */
			status = status & ( ~_mask );
		}
		else
		{
			/*
			 * Currently disabled.
			 */
			status = status | _mask;
		}
	}

	m = ctx->message_processor->create_set_pmic_status( this->board_id.toStdString(), status );
	ctx->send_message( m );
	this->update_data( );
	this->timer->start( DATA_UPDATE_TIMER );
}

void BOARD_INFO_WIDGET::cmd_enable_do_pmic_clicked( void )
{
	this->manage_pmic_status( PMIC_DO_EN_MASK );
	return;
}

void BOARD_INFO_WIDGET::cmd_enable_ai_pmic_clicked( void )
{
	this->manage_pmic_status( PMIC_AI_EN_MASK );
	return;
}

void BOARD_INFO_WIDGET::cmd_reset_do_pmic_clicked( void )
{
	this->manage_pmic_status( PMIC_DO_ERR_MASK );
	return;
}

void BOARD_INFO_WIDGET::cmd_reset_ai_pmic_clicked( void )
{
	this->manage_pmic_status( PMIC_AI_ERR_MASK );
	return;
}

void BOARD_INFO_WIDGET::cmd_enable_do_clicked( int _do )
{
	this->timer->stop( );
	MESSAGE_PTR m;// = this->update_data_and_return( );
	const vector<string>& parts = m->get_parts( );
	IOCOMM::DO_CACHE_ENTRY do_value( parts[AI_COUNT] );
	uint8_t mask;
	uint8_t status = do_value.get_value( );

	switch ( _do )
	{
		case 0:
			mask = DO_1_MASK;
			break;

		case 1:
			mask = DO_2_MASK;
			break;

		case 2:
			mask = DO_3_MASK;
			break;

		case 3:
			mask = DO_4_MASK;
			break;
	}

	if ( status & mask )
	{
		/*
		 * Output is enabled
		 */
		status = status & ( ~mask );
	}
	else
	{
		/*
		 * Output is disabled
		 */
		status = status | mask;
	}

	m = ctx->message_processor->create_set_status( this->board_id.toStdString(), status );
	ctx->send_message( m );
	this->update_data( );
	this->timer->start( DATA_UPDATE_TIMER );
}