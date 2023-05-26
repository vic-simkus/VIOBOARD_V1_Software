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
#ifndef __LOGIC_TABLE__
#define __LOGIC_TABLE__

#include <QTableWidget>
#include <QStringList>
class LOGIC_TABLE : public QTableWidget
{
		Q_OBJECT;
	public:
		LOGIC_TABLE( QWidget* _parent, const QStringList& _columns );
		LOGIC_TABLE( QWidget* _parent );
	protected:
	private:

	protected slots:
		void slot_set_headers( const QStringList& _columns );
		void slot_item_doubleclicked( QTableWidgetItem* );
		void slot_init( void );

};

#endif
