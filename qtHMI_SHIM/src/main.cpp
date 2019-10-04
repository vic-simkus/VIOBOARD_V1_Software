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

#include <QApplication>
#include "windows/MAIN_WINDOW.h"

#include "globals.h"

uint16_t
checksum( uint16_t const data[], int nWords )
{
	uint32_t  sum = 0;

	/*
	 * IP headers always contain an even number of bytes.
	 */
	while ( nWords-- > 0 )
	{
		sum += *( data++ );
	}

	/*
	 * Use carries to compute 1's complement sum.
	 */
	sum = ( sum >> 16 ) + ( sum & 0xFFFF );
	sum += sum >> 16;
	/*
	 * Return the inverted 16-bit result.
	 */
	return ( ( unsigned short ) ~sum );
}   /* NetIpChecksum() */

int main( int argc, char* argv[] )
{
	GLOBALS::configure_logging( LOGGING::ENUM_LOG_LEVEL::DEBUG );
	GLOBALS::configure_signals( );
	QApplication app( argc, argv );
	MAIN_WINDOW main_window;
	main_window.show( );
	return app.exec( );
}
