#include "DEBUG_FORCE_WIDGET.h"

#include <QPushButton>
#include <QSpinBox>
#include <QLabel>

#include <QVBoxLayout>
#include <QHBoxLayout>

const QString DEBUG_FORCE_WIDGET::INDICATOR_COLOR = "rgb(0, 20, 237)";

DEBUG_FORCE_WIDGET::DEBUG_FORCE_WIDGET( const QString& _label, unsigned int _index ) : QFrame()
{
	this->setObjectName( "DFW" );
	this->setLineWidth( 1 );
	this->is_forced = false;
	this->index = _index;
	// Setup the widget frame
	this->setLineWidth( 1 );
	this->setFrameShape( Box );
	this->setFrameShadow( Sunken );
	// Create the child widgets
	this->label = new QLabel( "<b>" + _label, this );
	this->status = new QLabel( "not forced", this );
	this->cmd_reset = new QPushButton( "Reset", this );
	this->cmd_toggle = new QPushButton( "Toggle", this );
	// Create and molest the spin box
	this->offset = new QSpinBox( this );
	this->offset->setMinimum( 0 );
	this->offset->setMaximum( 4096 );
	this->offset->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Fixed );
	QVBoxLayout* main_layout = new QVBoxLayout();
	QHBoxLayout* temp_layout = new QHBoxLayout();
	this->setLayout( main_layout );
	this->layout()->addWidget( this->label );
	this->layout()->addWidget( this->offset );
	this->layout()->addWidget( cmd_reset );
	this->layout()->addWidget( cmd_toggle );
	temp_layout->addItem( new QSpacerItem( 1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed ) );
	temp_layout->addWidget( status );
	temp_layout->addItem( new QSpacerItem( 1, 1 , QSizePolicy::MinimumExpanding, QSizePolicy::Fixed ) );
	this->layout()->addItem( temp_layout );
	this->layout()->setContentsMargins( 4, 4, 4, 4 );
	// Connect signals
	connect( this->cmd_toggle, SIGNAL( clicked() ), this, SLOT( cmd_toggle_clicked() ) );
	connect( this->cmd_reset, SIGNAL( clicked() ), this, SLOT( cmd_reset_clicked() ) );
	return;
}

DEBUG_FORCE_WIDGET::~DEBUG_FORCE_WIDGET()
{
}

void DEBUG_FORCE_WIDGET::update_widget( void )
{
	if ( this->is_forced )
	{
		this->status->setText( "FORCED" );
		/*
		What a hacky way of doing this...
		*/
		this->setStyleSheet( " #DFW {border-color: " + DEBUG_FORCE_WIDGET::INDICATOR_COLOR + "; border-style: solid; border-width: 1px;}" );
		this->status->setStyleSheet( "color: " + DEBUG_FORCE_WIDGET::INDICATOR_COLOR + "; font-weight: bold;" );
	}
	else
	{
		this->status->setText( "not forced" );
		this->setStyleSheet( "" );
		this->status->setStyleSheet( "" );
	}
}

void DEBUG_FORCE_WIDGET::slot_update_real_value( uint16_t _value )
{
	if ( this->is_forced )
	{
		return;
	}

	this->offset->setValue( _value );
}
void DEBUG_FORCE_WIDGET::cmd_reset_clicked( void )
{
	this->offset->setValue( 0 );
	this->is_forced = false;
	this->update_widget();
	sig_reset_clicked( this->index );
}
void DEBUG_FORCE_WIDGET::cmd_toggle_clicked( void )
{
	if ( this->is_forced )
	{
		this->is_forced = false;
	}
	else
	{
		this->is_forced = true;
	}

	this->update_widget();
	this->sig_toggle_clicked( this->index, this->is_forced, this->offset->value() );
}