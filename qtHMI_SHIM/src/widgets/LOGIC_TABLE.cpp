/*
Copyright (C) 2019  Vidas Simkus (vic.simkus@simkus.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "LOGIC_TABLE.h"

#include <QHeaderView>

LOGIC_TABLE::LOGIC_TABLE( QWidget* _parent, const QStringList& _columns ) : QTableWidget( _parent )
{
	this->slot_init();
	this->slot_set_headers( _columns );

	return;
}

LOGIC_TABLE::LOGIC_TABLE( QWidget* _parent ) : QTableWidget( _parent )
{
	this->slot_init();

	return;
}

void LOGIC_TABLE::slot_set_headers( const QStringList& _columns )
{
	this->setColumnCount( _columns.count( ) );
	this->setHorizontalHeaderLabels( _columns );

	return;
}
void LOGIC_TABLE::slot_item_doubleclicked( QTableWidgetItem* _item )
{
	_item->tableWidget()->setCurrentItem( nullptr , QItemSelectionModel::Clear );

	return;

}
void LOGIC_TABLE::slot_init( void )
{
	this->setSelectionBehavior( QAbstractItemView::SelectRows );
	this->setSelectionMode( QAbstractItemView::SingleSelection );
	this->resizeColumnsToContents( );
	this->setEditTriggers( QAbstractItemView::NoEditTriggers );
	this->setAlternatingRowColors( true );
	this->verticalHeader()->setVisible( false );

	connect( this, SIGNAL( itemDoubleClicked( QTableWidgetItem* ) ), this, SLOT( slot_item_doubleclicked( QTableWidgetItem* ) ) );

	return;

}