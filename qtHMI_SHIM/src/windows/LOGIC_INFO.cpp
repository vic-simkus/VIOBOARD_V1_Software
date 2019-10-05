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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QLabel>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QSplitterHandle>
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
	// Temp variable
	QFrame* temp_frame = nullptr;
	/*
	 * Setup the set point widgets
	 */
	temp_frame = new QFrame( nullptr );
	temp_frame->setLayout( new QVBoxLayout( temp_frame ) );
	temp_frame->layout( )->addWidget( new QLabel( "<b>Set Points" ) );
	temp_frame->layout( )->addWidget( ( this->table_sp_points = new QTableWidget( this->group_logic_points ) ) );
	this->splitter_logic_points->addWidget( temp_frame );
	/*
	 * Setup the point value widgets
	 */
	temp_frame = new QFrame( nullptr );
	temp_frame->setLayout( new QVBoxLayout( temp_frame ) );
	temp_frame->layout( )->addWidget( new QLabel( "<b>Point Values" ) );
	temp_frame->layout( )->addWidget( ( this->table_point_values = new QTableWidget( this->group_logic_points ) ) );
	this->splitter_logic_points->addWidget( temp_frame );
	/*
	 * Setup the DO widgets
	 */
	temp_frame = new QFrame( this->group_io_points );
	temp_frame->setLayout( new QVBoxLayout( temp_frame ) );
	temp_frame->layout( )->addWidget( new QLabel( "<b>DO Points" ) );
	temp_frame->layout( )->addWidget( this->table_do_points = new QTableWidget( nullptr ) );
	this->splitter_io_points->addWidget( temp_frame );
	/*
	 * Setup the AI widgets
	 */
	temp_frame = new QFrame( this->group_io_points );
	temp_frame->setLayout( new QVBoxLayout( temp_frame ) );
	temp_frame->layout( )->addWidget( new QLabel( "<b>AI Points" ) );
	temp_frame->layout( )->addWidget( this->table_ai_points = new QTableWidget( nullptr ) );
	this->splitter_io_points->addWidget( temp_frame );
	/*
	 * Setup the MAP widgets
	 */
	temp_frame = new QFrame( this->group_point_map );
	temp_frame->setLayout( new QVBoxLayout( temp_frame ) );
	temp_frame->layout( )->addWidget( this->table_map_points = new QTableWidget( nullptr ) );
	this->group_point_map->layout()->addWidget( temp_frame );
	/*
	 * Setup the TableWidget stuffs.
	 */
	QStringList table_column_headers;
	table_column_headers.append( "Board" );
	table_column_headers.append( "#" );
	table_column_headers.append( "Description" );
	this->table_do_points->setColumnCount( table_column_headers.count( ) );
	this->table_ai_points->setColumnCount( table_column_headers.count( ) );
	this->table_do_points->setHorizontalHeaderLabels( table_column_headers );
	this->table_ai_points->setHorizontalHeaderLabels( table_column_headers );
	table_column_headers.clear( );
	table_column_headers.append( "Set Point Name" );
	table_column_headers.append( "Set Point Value" );
	this->table_sp_points->setColumnCount( table_column_headers.count( ) );
	this->table_sp_points->setHorizontalHeaderLabels( table_column_headers );
	table_column_headers.clear( );
	table_column_headers.append( "Point Name" );
	table_column_headers.append( "Point Value" );
	this->table_point_values->setColumnCount( table_column_headers.count( ) );
	this->table_point_values->setHorizontalHeaderLabels( table_column_headers );
	table_column_headers.clear( );
	table_column_headers.append( "Map Name" );
	table_column_headers.append( "Board" );
	table_column_headers.append( "Type" );
	table_column_headers.append( "#" );
	table_column_headers.append( "Description" );
	this->table_map_points->setColumnCount( table_column_headers.count( ) );
	this->table_map_points->setHorizontalHeaderLabels( table_column_headers );
	/*
	 * Global table properties
	 */
	this->table_do_points->setSelectionBehavior( QAbstractItemView::SelectRows );
	this->table_ai_points->setSelectionBehavior( QAbstractItemView::SelectRows );
	this->table_map_points->setSelectionBehavior( QAbstractItemView::SelectRows );
	this->table_sp_points->setSelectionBehavior( QAbstractItemView::SelectRows );
	this->table_point_values->setSelectionBehavior( QAbstractItemView::SelectRows );
	this->table_do_points->resizeColumnsToContents( );
	this->table_ai_points->resizeColumnsToContents( );
	this->table_map_points->resizeColumnsToContents( );
	this->table_sp_points->resizeColumnsToContents( );
	this->table_point_values->resizeColumnsToContents( );
	this->table_do_points->setEditTriggers( QAbstractItemView::NoEditTriggers );
	this->table_ai_points->setEditTriggers( QAbstractItemView::NoEditTriggers );
	this->table_map_points->setEditTriggers( QAbstractItemView::NoEditTriggers );
	this->table_sp_points->setEditTriggers( QAbstractItemView::NoEditTriggers );
	this->table_point_values->setEditTriggers( QAbstractItemView::NoEditTriggers );
	this->table_do_points->setAlternatingRowColors( true );
	this->table_ai_points->setAlternatingRowColors( true );
	this->table_map_points->setAlternatingRowColors( true );
	this->table_sp_points->setAlternatingRowColors( true );
	this->table_point_values->setAlternatingRowColors( true );
	this->table_do_points->verticalHeader()->setVisible( false );
	this->table_ai_points->verticalHeader()->setVisible( false );
	this->table_map_points->verticalHeader()->setVisible( false );
	this->table_sp_points->verticalHeader()->setVisible( false );
	this->table_point_values->verticalHeader()->setVisible( false );
	/*
	 * Set splitter ratios
	 */
	this->splitter_main_window->setStretchFactor( 0, 6 );
	this->splitter_main_window->setStretchFactor( 1, 1 );
	this->splitter_main_window->setStretchFactor( 2, 6 );
	/*
	Setup splitter handles.
	*/
	this->setup_splitter_handle( this->splitter_main_window );
	this->setup_splitter_handle( this->splitter_io_points );
	this->setup_splitter_handle( this->splitter_logic_points );
	/*
	 * Finished with the UI setup/generation
	 */
	ctx = BBB_HVAC::CLIENT::CLIENT_CONTEXT::create_instance( );
	ctx->connect( );
	this->timer = new QTimer( this );
	connect( this->timer, SIGNAL( timeout( ) ), this, SLOT( slot_update_labels( ) ) );
	this->timer->setSingleShot( true );
	this->timer->start( 500 );
}

LOGIC_INFO::~LOGIC_INFO( )
{
}

void LOGIC_INFO::setup_splitter_handle( QSplitter* _splitter )
{
	Q_ASSERT( _splitter != NULL );
	_splitter->setHandleWidth( 10 );

	for ( int i = 1; i < _splitter->count() ; i++ )
	{
		QSplitterHandle* handle = _splitter->handle( i );
		Q_ASSERT( handle != NULL );
		QLayout* handle_layout = nullptr;
		QFrame* line = new QFrame();
		line->setFrameShadow( QFrame::Raised );
		line->setLineWidth( 3 );
		line->setMidLineWidth( 1 );

		if ( _splitter->orientation() == Qt::Horizontal )
		{
			// If the splitter layout is horizontal then the splitter handle will be vertical
			handle_layout = new QVBoxLayout();
			line->setFrameShape( QFrame::VLine );
		}
		else
		{
			// splitter layout is vertical therefore the handle will be horizontal
			handle_layout = new QHBoxLayout();
			line->setFrameShape( QFrame::HLine );
		}

		handle_layout->addWidget( line );
		handle->setLayout( handle_layout );
		handle_layout->setSpacing( 0 );
		handle_layout->setMargin( 0 );
	}

	return;
}

void LOGIC_INFO::dump_message_parts( BBB_HVAC::MESSAGE_PTR& _message )
{
	for ( size_t part_idx = 0; part_idx < _message->get_part_count( ); part_idx++ )
	{
		std::string part = _message->get_part_as_s( part_idx );
		LOG_DEBUG_STAT( "Part [" + num_to_str( part_idx ) + "]: " + part );
	}
}

static void populate_table_widget( QTableWidget* _table_widget, const BBB_HVAC::MESSAGE_PTR& _message )
{
	_table_widget->clearContents( );
	_table_widget->setRowCount( _message->get_part_count( ) );

	for ( size_t i = 0; i < _message->get_part_count( ); i++ )
	{
		BBB_HVAC::BOARD_POINT board_point = BBB_HVAC::BOARD_POINT::from_string( _message->get_part_as_s( i ) );
		_table_widget->setItem( i, 0, new QTableWidgetItem( QString( board_point.get_board_tag( ).data( ) ) ) );
		_table_widget->setItem( i, 1, new QTableWidgetItem( QString::number( board_point.get_point_id( ) ) ) );
		_table_widget->setItem( i, 2, new QTableWidgetItem( QString( board_point.get_description( ).data( ) ) ) );
	}

	_table_widget->resizeColumnsToContents( );
}

void LOGIC_INFO::slot_update_labels( void )
{
	LOG_DEBUG_STAT( "Updating labels." );
	BBB_HVAC::MESSAGE_PTR message;
	message = this->ctx->message_processor->create_get_labels_message_request( BBB_HVAC::ENUM_CONFIG_TYPES::DO );
	message = this->ctx->send_message_and_wait( message );
	LOG_DEBUG_STAT( "DO labels:" );
	this->dump_message_parts( message );
	populate_table_widget( this->table_do_points, message );
	message = this->ctx->message_processor->create_get_labels_message_request( BBB_HVAC::ENUM_CONFIG_TYPES::AI );
	message = this->ctx->send_message_and_wait( message );
	LOG_DEBUG_STAT( "AI labels:" );
	this->dump_message_parts( message );
	populate_table_widget( this->table_ai_points, message );
	message = this->ctx->message_processor->create_get_labels_message_request( BBB_HVAC::ENUM_CONFIG_TYPES::MAP );
	message = this->ctx->send_message_and_wait( message );
	LOG_DEBUG_STAT( "MAP labels:" );
	this->dump_message_parts( message );
	std::map<std::string, std::string> map;
	BBB_HVAC::MESSAGE::message_to_map( message, map );
	this->table_map_points->clearContents( );
	this->table_map_points->setRowCount( map.size( ) );
	int row_index = 0;

	for ( auto map_iterator = map.cbegin( ); map_iterator != map.cend( ); ++map_iterator )
	{
		LOG_DEBUG_STAT( map_iterator->second );
		BBB_HVAC::BOARD_POINT board_point = BBB_HVAC::BOARD_POINT::from_string( map_iterator->second );
		this->table_map_points->setItem( row_index, 0, new QTableWidgetItem( QString( map_iterator->first.data( ) ) ) );
		this->table_map_points->setItem( row_index, 1, new QTableWidgetItem( QString( board_point.get_board_tag( ).data( ) ) ) );
		this->table_map_points->setItem( row_index, 2, new QTableWidgetItem( QString( BBB_HVAC::CONFIG_ENTRY::type_to_string( board_point.get_type( ) ).data( ) ) ) );
		this->table_map_points->setItem( row_index, 3, new QTableWidgetItem( QString::number( board_point.get_point_id( ) ) ) );
		this->table_map_points->setItem( row_index, 4, new QTableWidgetItem( QString( board_point.get_description( ).data( ) ) ) );
		row_index += 1;
	}

	this->table_map_points->resizeColumnsToContents( );
	this->update_status( );
	disconnect( this->timer, SIGNAL( timeout( ) ), this, SLOT( slot_update_labels( ) ) );
	connect( this->timer, SIGNAL( timeout( ) ), this, SLOT( slot_update_data_timer( ) ) );
	this->timer->setSingleShot( false );
	this->timer->start( 1000 );
	return;
}

void LOGIC_INFO::update_status( void )
{
	//LOG_DEBUG_STAT( "Updating logic status." );
	/*
	 * Update the logic point values
	 */
	int current_row = this->table_point_values->currentRow( );
	BBB_HVAC::MESSAGE_PTR message;
	unsigned int row_idx = 0;
	message = this->ctx->message_processor->create_read_logic_status( );
	message = this->ctx->send_message_and_wait( message );
	std::map<std::string, std::string> map;
	BBB_HVAC::MESSAGE::message_to_map( message, map );
	this->table_point_values->clearContents( );
	this->table_point_values->setRowCount( map.size( ) );
	row_idx = 0;

	for ( auto map_iterator = map.begin( ); map_iterator != map.end( ); ++map_iterator )
	{
		this->table_point_values->setItem( row_idx, 0, new QTableWidgetItem( QString( map_iterator->first.data( ) ) ) );
		this->table_point_values->setItem( row_idx, 1, new QTableWidgetItem( QString( map_iterator->second.data( ) ) ) );
		row_idx += 1;
	}

	this->table_point_values->resizeColumnsToContents( );
	this->table_point_values->setCurrentCell( current_row, 0 );
	/*
	 * Update the set point values
	 */
	current_row = this->table_sp_points->currentRow( );
	message = this->ctx->message_processor->create_get_labels_message_request( BBB_HVAC::ENUM_CONFIG_TYPES::SP );
	message = this->ctx->send_message_and_wait( message );
	this->table_sp_points->clearContents( );
	this->table_sp_points->setRowCount( message->get_part_count( ) );

	for ( unsigned int row_idx = 0; row_idx < message->get_part_count( ); ++row_idx )
	{
		std::string message_part = message->get_part_as_s( row_idx );
		//LOG_DEBUG_STAT(message_part);
		BBB_HVAC::SET_POINT sp = BBB_HVAC::SET_POINT::from_string( message_part );
		this->table_sp_points->setItem( row_idx, 0, new QTableWidgetItem( QString( sp.get_description( ).data( ) ) ) );
		this->table_sp_points->setItem( row_idx, 1, new QTableWidgetItem( QString::number( sp.get_value( ) ) ) );
	}

	this->table_sp_points->resizeColumnsToContents( );
	this->table_sp_points->setCurrentCell( current_row, 0 );
	return;
}

void LOGIC_INFO::slot_update_data_timer( void )
{
	this->sig_update_start( );
	this->update_status( );
	this->sig_update_finish( );
}
