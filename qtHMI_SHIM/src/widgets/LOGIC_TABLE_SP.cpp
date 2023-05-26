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

#include "widgets/LOGIC_TABLE_SP.h"

#include <QStringList>

LOGIC_TABLE_SP::LOGIC_TABLE_SP( QWidget* _parent ) : LOGIC_TABLE( _parent )
{
	QStringList headers;
	/*
	 * Setup the set point widgets
	 */
	headers.clear( );
	headers.append( "Name" );
	headers.append( "Value" );
	headers.append( "New Value" );

	this->slot_set_headers( headers );

	return;
}