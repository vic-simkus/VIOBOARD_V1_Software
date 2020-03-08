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
#include <QMessageBox>
#include <QCommandLineParser>
#include <QStringList>

#include <iostream>
#include "windows/MAIN_WINDOW.h"

#include "globals.h"
#include "config.h"

#include "lib/bbb_hvac.hpp"

DEF_LOGGER_STAT( "MAIN" );

MESSAGE_BUS* message_bus;

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
	BBB_HVAC::GLOBALS::configure_logging( 1, BBB_HVAC::LOGGING::ENUM_LOG_LEVEL::DEBUG );
	BBB_HVAC::GLOBALS::configure_signals( );

	QApplication app( argc, argv );

	app.setApplicationName( APP_NAME );
	app.setApplicationVersion( APP_VERSION );

	QCommandLineParser parser;
	parser.setApplicationDescription( "QT5 based HMI shim to LOGIC_LOOP" );
	parser.addHelpOption();
	parser.addVersionOption();

	QCommandLineOption clo_domain_unix( "d", "Connect using a domain socket,  This is the default mode." );
	QCommandLineOption clo_domain_inet( "i", "Connect using a TCPIP socket." );
	QCommandLineOption clo_address( "a", "Remote address.  Relevant only if -i is specified.  Defaults to " + QString( GC_DEFAULT_LISTEN_INTERFACE ) + "." , "address", QString( GC_DEFAULT_LISTEN_INTERFACE ) );
	QCommandLineOption clo_port( "p", "Remote port.  Mandatory if -i is specified.  Relevant only if -i is specified.  Defaults to " + QString::number( GC_DEFAULT_TCPIP_PORT ) + "." , "port", QString::number( GC_DEFAULT_TCPIP_PORT ) );
	QCommandLineOption clo_board_list( "b", "Board list. Specify multiple times for multiple boards.  Defaults to BOARD1", "board_list", "BOARD1" );

	parser.addOption( clo_domain_unix );
	parser.addOption( clo_domain_inet );
	parser.addOption( clo_address );
	parser.addOption( clo_port );
	parser.addOption( clo_board_list );

	parser.process( app );

	bool domain_unix = parser.isSet( clo_domain_unix );
	bool domain_inet = parser.isSet( clo_domain_inet );
	uint16_t port = parser.value( clo_port ).toUInt();
	QString address = parser.value( clo_address );
	QStringList board_list = parser.values( clo_board_list );

	if ( !domain_unix && !domain_inet )
	{
		domain_unix = true;
	}

	if ( domain_unix && domain_inet )
	{
		std::cerr << "Confusing command line parameters.  Both -d and -i specified.  They are mutually exclusive" << endl;
		exit( -1 );
	}

	if ( domain_unix )
	{
		address = GC_LOCAL_COMMAND_SOCKET;
	}

	message_bus = new MESSAGE_BUS( 10, ( domain_unix ? BBB_HVAC::SOCKET_TYPE::DOMAIN : BBB_HVAC::SOCKET_TYPE::TCPIP ), address, port );

	try
	{
		message_bus->connect_to_remote();
	}
	catch ( const std::exception& e )
	{
		QMessageBox::critical( nullptr, "Connection failure", QString( e.what() ) + "\n*********\nStart the program [" + QString( argv[0] ) + "] with the '-h' command line parameter for help." );
		exit( -1 );
	}

	MAIN_WINDOW main_window( board_list );
	main_window.show( );
	app.exec( );


	delete message_bus;
}
