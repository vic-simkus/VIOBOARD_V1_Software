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

#include "LOGIC_INFO.h"
#include "ui_util.h"
#include "widgets/LOGIC_TABLE.h"
#include "widgets/LOGIC_TABLE_SP.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QSplitterHandle>
#include <QGroupBox>
#include <QListWidget>
#include <QTableWidget>
#include <QSplitter>

#include "globals.h"
DEF_LOGGER_STAT( "LOGIC_INFO" );

LOGIC_INFO::LOGIC_INFO( QWidget* _p ) : QFrame( _p )
{
	this->setLayout( new QVBoxLayout( this ) );
	this->splitter_main_window = new QSplitter( Qt::Vertical, this );
	this->layout( )->addWidget( this->splitter_main_window );

	// Create group splitters
	this->splitter_io_points = new QSplitter( Qt::Horizontal, nullptr );
	this->splitter_logic_points = new QSplitter( Qt::Horizontal, nullptr );

	// Create groups
	// Create IO point group
	this->group_io_points = new QGroupBox( "I/O Points", nullptr );
	this->group_io_points->setLayout( new QVBoxLayout( nullptr ) );
	this->group_io_points->layout( )->addWidget( this->splitter_io_points );
	// Create point map group
	this->group_point_map = new QGroupBox( "Point Mappings", nullptr );
	this->group_point_map->setLayout( new QVBoxLayout( nullptr ) );
	// Create logic point map
	this->group_logic_points = new QGroupBox( "Logic/Runtime Points", nullptr );
	this->group_logic_points->setLayout( new QVBoxLayout( nullptr ) );
	this->group_logic_points->layout( )->addWidget( this->splitter_logic_points );
	// Add groups to main window splitter
	this->splitter_main_window->addWidget( this->group_io_points );
	this->splitter_main_window->addWidget( this->group_point_map );
	this->splitter_main_window->addWidget( this->group_logic_points );

	/*
		Instantiate the tables containing the logic info.
	*/
	{

		// Temp variable
		QFrame* temp_frame = nullptr;
		QStringList table_column_headers;

		temp_frame = new QFrame( nullptr );
		temp_frame->setLayout( new QVBoxLayout( temp_frame ) );
		temp_frame->layout( )->addWidget( new QLabel( "<b>Set Points" ) );
		temp_frame->layout( )->addWidget( ( this->table_sp_points = new LOGIC_TABLE_SP( this->group_logic_points ) ) );
		this->splitter_logic_points->addWidget( temp_frame );

		/*
		 * Setup the point value widgets
		 */
		table_column_headers.clear( );
		table_column_headers.append( "Point Name" );
		table_column_headers.append( "Point Value" );

		temp_frame = new QFrame( nullptr );
		temp_frame->setLayout( new QVBoxLayout( temp_frame ) );
		temp_frame->layout( )->addWidget( new QLabel( "<b>Point Values" ) );
		temp_frame->layout( )->addWidget( ( this->table_point_values = new LOGIC_TABLE( this->group_logic_points, table_column_headers ) ) );
		this->splitter_logic_points->addWidget( temp_frame );

		/*
		 * Setup the DO widgets
		 */
		table_column_headers.clear( );
		table_column_headers.append( "Board" );
		table_column_headers.append( "#" );
		table_column_headers.append( "Description" );

		temp_frame = new QFrame( this->group_io_points );
		temp_frame->setLayout( new QVBoxLayout( temp_frame ) );
		temp_frame->layout( )->addWidget( new QLabel( "<b>DO Points" ) );
		temp_frame->layout( )->addWidget( this->table_do_points = new LOGIC_TABLE( nullptr, table_column_headers ) );
		this->splitter_io_points->addWidget( temp_frame );

		/*
		 * Setup the AI widgets
		 */

		// AI table has same columns as DO table

		temp_frame = new QFrame( this->group_io_points );
		temp_frame->setLayout( new QVBoxLayout( temp_frame ) );
		temp_frame->layout( )->addWidget( new QLabel( "<b>AI Points" ) );
		temp_frame->layout( )->addWidget( this->table_ai_points = new LOGIC_TABLE( nullptr , table_column_headers ) );
		this->splitter_io_points->addWidget( temp_frame );

		/*
		 * Setup the MAP widgets
		 */
		table_column_headers.clear( );
		table_column_headers.append( "Map Name" );
		table_column_headers.append( "Board" );
		table_column_headers.append( "Type" );
		table_column_headers.append( "#" );
		table_column_headers.append( "Description" );

		temp_frame = new QFrame( this->group_point_map );
		temp_frame->setLayout( new QVBoxLayout( temp_frame ) );
		temp_frame->layout( )->addWidget( this->table_map_points = new LOGIC_TABLE( nullptr , table_column_headers ) );
		this->group_point_map->layout()->addWidget( temp_frame );
	}

	/*
	 * Set splitter ratios
	 */
	this->splitter_main_window->setStretchFactor( 0, 6 );
	this->splitter_main_window->setStretchFactor( 1, 1 );
	this->splitter_main_window->setStretchFactor( 2, 6 );
	/*
	Setup splitter handles.
	*/
	setup_splitter_handle( this->splitter_main_window );
	setup_splitter_handle( this->splitter_io_points );
	setup_splitter_handle( this->splitter_logic_points );
	/*
	Connect the message bus listeners
	*/
	connect( message_bus, SIGNAL( sig_point_label_data( MESSAGE_BUS::COMMANDS , const QVector<QVector<QString>>& ) ), this, SLOT( slot_mb_label_data( MESSAGE_BUS::COMMANDS, const QVector<QVector<QString>>& ) ) );
	connect( message_bus, SIGNAL( sig_point_map_data( MESSAGE_BUS::COMMANDS , const QMap<QString, QVector<QString>>& ) ), this, SLOT( slot_mb_map_data( MESSAGE_BUS::COMMANDS, const QMap<QString, QVector<QString>>& ) ) );
	connect( message_bus, SIGNAL( sig_logic_status_data( MESSAGE_BUS::COMMANDS , const QMap<QString, QString>& ) ), this, SLOT( slot_mb_logic_status_update( MESSAGE_BUS::COMMANDS , const QMap<QString, QString>& ) ) );
	connect( message_bus, SIGNAL( sig_set_point_data( MESSAGE_BUS::COMMANDS , const QMap<QString, QString>& ) ), this, SLOT( slot_mb_set_point_update( MESSAGE_BUS::COMMANDS , const QMap<QString, QString>& ) ) );
	/*
	Send the label request messages.
	*/
	message_bus->add_message( MESSAGE_BUS::MESSAGE( MESSAGE_BUS::COMMANDS::GET_LABELS_DO ) );
	message_bus->add_message( MESSAGE_BUS::MESSAGE( MESSAGE_BUS::COMMANDS::GET_LABELS_AI ) );
	message_bus->add_message( MESSAGE_BUS::MESSAGE( MESSAGE_BUS::COMMANDS::GET_LABELS_MAP ) );

	return;
}

LOGIC_INFO::~LOGIC_INFO( )
{
}


void LOGIC_INFO::dump_message_parts( BBB_HVAC::MESSAGE_PTR& _message )
{
	for ( size_t part_idx = 0; part_idx < _message->get_part_count( ); part_idx++ )
	{
		std::string part = _message->get_part_as_s( part_idx );
		LOG_DEBUG( "Part [" + num_to_str( part_idx ) + "]: " + part );
	}
}

static void populate_table_widget( QTableWidget* _table_widget, const QVector<QVector<QString>> _data )
{
	_table_widget->clearContents( );
	_table_widget->setRowCount( _data.count( ) );
	unsigned int idx = 0;

	for ( auto i = _data.begin(); i != _data.end(); ++i )
	{
		_table_widget->setItem( idx, 0, new QTableWidgetItem( ( *i )[0] ) );
		_table_widget->setItem( idx, 1, new QTableWidgetItem( ( *i )[1] ) );
		_table_widget->setItem( idx, 2, new QTableWidgetItem( ( *i )[2] ) );
		idx += 1;
	}

	_table_widget->resizeColumnsToContents( );
}

void LOGIC_INFO::slot_mb_label_data( MESSAGE_BUS::COMMANDS _cmd, const QVector<QVector<QString>>& _data )
{
	if ( _cmd == MESSAGE_BUS::COMMANDS::GET_LABELS_DO )
	{
		//LOG_DEBUG( "DO labels" );
		populate_table_widget( this->table_do_points, _data );
	}
	else if ( _cmd == MESSAGE_BUS::COMMANDS::GET_LABELS_AI )
	{
		//LOG_DEBUG( "AI labels" );
		populate_table_widget( this->table_ai_points, _data );
	}
	else
	{
		LOG_ERROR( "Unrecognized command: " + num_to_str( ( int )_cmd ) );
	}
}
void LOGIC_INFO::slot_mb_map_data( MESSAGE_BUS::COMMANDS, const QMap<QString, QVector<QString>>& _data )
{
	//LOG_DEBUG( "MAP labels:" );
	this->table_map_points->clearContents( );
	this->table_map_points->setRowCount( _data.size( ) );
	int row_index = 0;

	for ( auto map_iterator = _data.constBegin( ); map_iterator != _data.constEnd( ); ++map_iterator )
	{
		const QVector<QString>& value = map_iterator.value();
		this->table_map_points->setItem( row_index, 0, new QTableWidgetItem( QString( map_iterator.key( ) ) ) );
		this->table_map_points->setItem( row_index, 1, new QTableWidgetItem( value[0] ) );
		this->table_map_points->setItem( row_index, 2, new QTableWidgetItem( value[1] ) );
		this->table_map_points->setItem( row_index, 3, new QTableWidgetItem( value[2] ) );
		this->table_map_points->setItem( row_index, 4, new QTableWidgetItem( value[3] ) );
		row_index += 1;
	}

	this->table_map_points->resizeColumnsToContents( );
}

void LOGIC_INFO::slot_mb_logic_status_update( MESSAGE_BUS::COMMANDS, const QMap<QString, QString>& _data )
{
	QList<QTableWidgetItem*> selected_items = this->table_point_values->selectedItems();

	auto current_row = -1;

	if ( selected_items.count() == 2 )
	{
		current_row = selected_items.at( 0 )->row();
	}

	QString current_id;

	if ( current_row >= 0 )
	{
		QTableWidgetItem* item = this->table_point_values->item( current_row, 0 );
		current_id = item->text();
	}


	this->table_point_values->clearContents( );
	this->table_point_values->setRowCount( _data.count( ) );
	int row_idx = 0;

	for ( auto map_iterator = _data.constBegin( ); map_iterator != _data.constEnd( ); ++map_iterator )
	{
		this->table_point_values->setItem( row_idx, 0, new QTableWidgetItem( map_iterator.key() ) );
		this->table_point_values->setItem( row_idx, 1, new QTableWidgetItem( map_iterator.value() ) );
		row_idx += 1;
	}

	this->table_point_values->resizeColumnsToContents( );
	//this->table_point_values->setCurrentCell( row_idx, 0 );

	if ( current_row >= 0 )
	{
		QList<QTableWidgetItem*> search_results = this->table_point_values->findItems( current_id, Qt::MatchFixedString | Qt::MatchCaseSensitive );

		if ( search_results.count() != 1 )
		{
			LOG_INFO( "Previously selected item [" + current_id.toStdString() + "] was not found." );
		}
		else
		{
			this->table_point_values->setCurrentItem( search_results.at( 0 ) );
		}
	}
}

void LOGIC_INFO::slot_mb_set_point_update( MESSAGE_BUS::COMMANDS, const QMap<QString, QString>& _data )
{

	int current_row = this->table_sp_points->currentRow( );
	this->table_sp_points->clearContents( );
	this->table_sp_points->setRowCount( _data.count() );
	unsigned int row_idx = 0;

	for ( auto mi = _data.begin(); mi != _data.end(); ++mi )
	{
		this->table_sp_points->setItem( row_idx, 0, new QTableWidgetItem( mi.key() ) );
		this->table_sp_points->setItem( row_idx, 1, new QTableWidgetItem( mi.value() ) );
		row_idx += 1;
	}

	this->table_sp_points->resizeColumnsToContents( );
	this->table_sp_points->setCurrentCell( current_row, 0 );
	return;
}