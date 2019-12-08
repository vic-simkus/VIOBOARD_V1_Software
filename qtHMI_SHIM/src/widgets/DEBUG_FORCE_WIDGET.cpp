#include "DEBUG_FORCE_WIDGET.h"

#include <QPushButton>
#include <QSpinBox>
#include <QLabel>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QComboBox>

const QString DEBUG_FORCE_WIDGET::INDICATOR_COLOR = "rgb(0, 20, 237)";

DEBUG_FORCE_WIDGET::DEBUG_FORCE_WIDGET( const QString& _label, unsigned int _index ) : QFrame()
{

	QVBoxLayout* main_layout = new QVBoxLayout();


	this->mult_select = new QComboBox( this );
	mult_select->addItem( "1" );
	mult_select->addItem( "10" );
	mult_select->addItem( "100" );
	mult_select->addItem( "1000" );

	this->setLayout( main_layout );

	this->txt_label = _label;

	this->setObjectName( "DFW" );
	this->setLineWidth( 1 );
	this->is_forced = false;
	this->index = _index;
	// Setup the widget frame
	this->setLineWidth( 1 );
	this->setFrameShape( Box );
	this->setFrameShadow( Sunken );
	// Create the child widgets
	this->label = new QLabel( "<b>" + this->txt_label, this );

	this->cmd_toggle = new QPushButton( "Force", this );
	// Create and molest the spin box
	this->offset = new QSpinBox( this );
	this->offset->setMinimum( 0 );
	this->offset->setMaximum( 4096 );
	this->offset->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Fixed );


	QHBoxLayout* temp_layout = new QHBoxLayout();

	temp_layout->addItem( new QSpacerItem( 1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed ) );

	temp_layout->addItem( new QSpacerItem( 1, 1 , QSizePolicy::MinimumExpanding, QSizePolicy::Fixed ) );

	this->layout()->setContentsMargins( 4, 4, 4, 4 );

	this->layout()->addWidget( this->label );
	this->layout()->addWidget( this->offset );
	this->layout()->addWidget( this->mult_select );

	this->layout()->addItem( temp_layout );
	this->layout()->addWidget( cmd_toggle );

	// Connect signals
	connect( this->cmd_toggle, SIGNAL( clicked() ), this, SLOT( cmd_toggle_clicked() ) );
	connect( this->offset, SIGNAL( valueChanged( int ) ), this, SLOT( slot_spinner_changed( ) ) );
	connect( this->mult_select, SIGNAL( activated( int ) ), this, SLOT( slot_mult_changed( int ) ) );

	this->update_widget();
	return;
}
void DEBUG_FORCE_WIDGET::slot_mult_changed( int _idx )
{
	if ( _idx < 1 )
	{
		this->offset->setSingleStep( 1 );
	}
	else if ( _idx == 1 )
	{
		this->offset->setSingleStep( 10 );
	}
	else if ( _idx == 2 )
	{
		this->offset->setSingleStep( 100 );
	}
	else if ( _idx == 3 )
	{
		this->offset->setSingleStep( 1000 );
	}
	else if ( _idx == 4 )
	{
		this->offset->setSingleStep( 10000 );
	}

	return;
}
DEBUG_FORCE_WIDGET::~DEBUG_FORCE_WIDGET()
{
}

void DEBUG_FORCE_WIDGET::slot_spinner_changed( void )
{
	if ( this->is_forced )
	{
		this->sig_force_clicked( this->index, this->is_forced, this->offset->value() );
	}
}
void DEBUG_FORCE_WIDGET::update_widget( void )
{
	if ( this->is_forced )
	{
		this->cmd_toggle->setText( "Unforce" );
		this->label->setText( "<b>" + this->txt_label + "(F)" );
		this->offset->setEnabled( true );
		this->mult_select->setEnabled( true );

		/*
		What a hacky way of doing this...
		*/
		this->setStyleSheet( " #DFW {border-color: " + DEBUG_FORCE_WIDGET::INDICATOR_COLOR + "; border-style: solid; border-width: 1px;}" );
		this->label->setStyleSheet( "color: " + DEBUG_FORCE_WIDGET::INDICATOR_COLOR + "; font-weight: bold;" );
	}
	else
	{
		this->cmd_toggle->setText( "Force" );
		this->label->setText( "<b>" + this->txt_label );
		this->offset->setEnabled( false );
		this->mult_select->setEnabled( false );
		this->setStyleSheet( "" );
		this->label->setStyleSheet( "" );
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

	this->sig_force_clicked( this->index, this->is_forced, this->offset->value() );
}