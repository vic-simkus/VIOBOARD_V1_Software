#include "MAIN_WIDGET.h"


#include "../windows/BOARD_INFO_WIDGET.h"
#include "../windows/RAW_BOARD_INFO.h"
#include "../windows/LOGIC_INFO.h"
#include "../windows/DEBUG_FRAME.h"

#include "ui_util.h"


#include <QTabWidget>
#include <QVBoxLayout>
#include <QSplitter>

MAIN_WIDGET::MAIN_WIDGET( QWidget* _parent ) : QFrame( _parent )
{

	this->setLayout( new QVBoxLayout() );

	QWidget* logic_widget = nullptr;

	QSplitter* main_splitter = new QSplitter( Qt::Vertical, this );
	this->layout()->addWidget( main_splitter );

	QTabWidget* tab_widget = new QTabWidget( this );

	tab_widget->addTab( ( logic_widget = new LOGIC_INFO( nullptr ) ), "LOGIC INFO" );
	tab_widget->addTab( new RAW_BOARD_INFO( nullptr ), "RAW BOARD INFO" );

	main_splitter->addWidget( tab_widget );

	DEBUG_FRAME* debug_frame = new DEBUG_FRAME( nullptr );

	main_splitter->addWidget( debug_frame );

	setup_splitter_handle( main_splitter );


	return;
}