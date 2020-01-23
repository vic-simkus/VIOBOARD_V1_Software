#include "command_line_parms.h"

#include <iostream>

COMMAND_LINE_PARMS::COMMAND_LINE_PARMS( size_t _argc, const char** _argv )
{
	this->argc = _argc;
	this->argv = _argv;

	this->verbose_flag = false;
	this->server_mode = false;

	address = "";
	port_s = "";

	port_i =  GC_DEFAULT_TCPIP_PORT;

	this->st = BBB_HVAC::SOCKET_TYPE::NONE;

	this->exe = string( argv[0] );

	this->log_file = "/var/log/BBB_HVAC.log";

	return;
}

void COMMAND_LINE_PARMS::process( void )
{
	for ( size_t i = 1; i < argc; i++ )
	{
		string p = trimmed( argv[i] );

		if ( p == "-d" )
		{
			if ( st != BBB_HVAC::SOCKET_TYPE::NONE )
			{
				print_cmd_error( exe, "Confusing command line.  Connection method specified more than once?" );
			}

			st = BBB_HVAC::SOCKET_TYPE::DOMAIN;
		}
		else if ( p == "-i" )
		{
			if ( st != BBB_HVAC::SOCKET_TYPE::NONE )
			{
				print_cmd_error( exe, "Confusing command line.  Connection method specified more than once?" );
			}

			st = BBB_HVAC::SOCKET_TYPE::TCPIP;
		}
		else if ( p == "-h" )
		{
			print_help( exe );
			exit( 0 );
		}
		else if ( p == "-v" )
		{
			verbose_flag = true;
		}
		else if ( p == "-p" )
		{
			if ( i == ( argc - 1 ) )
			{
				print_cmd_error( exe, "No port specified for the -p parameter." );
			}
			else
			{
				port_s = string( argv[i + 1] );
				i += 1;
			}
		}
		else if ( p == "-a" )
		{
			if ( i == ( argc - 1 ) )
			{
				print_cmd_error( exe, "No address specified for the -a parameter." );
			}
			else
			{
				address = string( argv[i + 1] );
				i += 1;
			}
		}
		else if ( p == "-s" )
		{
			server_mode = true;
		}
		else if ( p == "-l" )
		{
			if ( i == ( argc - 1 ) )
			{
				print_cmd_error( exe, "No log file specified for the -l parameter." );
			}
			else
			{
				this->log_file = string( argv[i + 1] );
				i += 1;
			}
		}
		else
		{
			print_cmd_error( exe, "Unrecognized command line parameter: " + p );
		}
	}

	if ( st == BBB_HVAC::SOCKET_TYPE::NONE )
	{
		print_cmd_error( exe, "Need to specify connection method.  See {-d|-i} parameters." );
	}

	switch ( st )
	{
		case BBB_HVAC::SOCKET_TYPE::NONE:
			// do nothing.
			break;

		case BBB_HVAC::SOCKET_TYPE::DOMAIN:
			if ( address.length() == 0 )
			{
				address = GC_LOCAL_COMMAND_SOCKET;
			}

			break;

		case BBB_HVAC::SOCKET_TYPE::TCPIP:
			if ( address.length() == 0 )
			{
				address = GC_DEFAULT_LISTEN_INTERFACE;
			}

			break;
	}

	if ( verbose_flag )
	{
		cout << "Verbose flag (-v) has been set via command line." << endl;
		cout << "    Connection type {-d|-i}: ";

		switch ( st )
		{
			case BBB_HVAC::SOCKET_TYPE::NONE:
				cout << "NONE";
				break;

			case BBB_HVAC::SOCKET_TYPE::DOMAIN:
				cout << "DOMAIN";
				break;

			case BBB_HVAC::SOCKET_TYPE::TCPIP:
				cout << "TCPIP";
				break;
		}

		cout << endl;

		cout << "    Address [-a]: [" << address << "]" << endl;
		cout << "    Port [-p]: [" << port_i << "]" << endl;
		cout << "    Server mode [-s]: [" << ( server_mode ? "TRUE" : "FALSE" ) << "]" << endl;
	}

}

bool COMMAND_LINE_PARMS::is_verbose_flag( void ) const
{
	return this->verbose_flag;
}

bool COMMAND_LINE_PARMS::is_server_mode( void ) const
{
	return this->server_mode;
}

const string& COMMAND_LINE_PARMS::get_address( void ) const
{
	return this->address;
}

uint16_t COMMAND_LINE_PARMS::get_port( void ) const
{
	return this->port_i;
}

BBB_HVAC::SOCKET_TYPE COMMAND_LINE_PARMS::get_socket_type( void ) const
{
	return this->st;
}

const string& COMMAND_LINE_PARMS::get_exe( void ) const
{
	return this->exe;
}

const string& COMMAND_LINE_PARMS::get_log_file( void ) const
{
	return this->log_file;
}

void COMMAND_LINE_PARMS::print_help( const string& _c )
{
	cout << "Usage: " << endl;
	cout << _c << " {-d|i} [-a <address>] [-p <port>] [-s] [-v]" << endl;
	cout << "Where: " << endl;
	cout << "\t-d - Listen on domain socket (mutually exclusive with -i)" << endl;
	cout << "\t-i - Listen on TCP/IP socket (mutually exclusive with -d)" << endl;
	cout << "\t-a <address> - Address to listen on.  File name (optional) if -d is specified.  Interface to bind to if -i is specified." << endl;
	cout << "\t  For -i, the interface should be specified in x.x.x.x notation. Default is 127.0.0.1." << endl;
	cout << "\t-p <port> - Port to listen to.  Relevant only if -i is specified.  Defaults to 6666" << endl;
	cout << "\t-l <log> - Log file.  Defaults to /var/log/BBB_HVAC.log";
	cout << "\t-s - Server mode.  If not specified application runs in console." << endl;
	cout << "\t-v - Verbose mode.  Produces extra debugging information to the console." << endl;
	cout << "\t-h - This help" << endl;
	exit( -1 );

	return;
}

void COMMAND_LINE_PARMS::print_cmd_error( const string& _c, const string& _failure )
{
	cerr << "Command line error: " << endl;
	cerr << _failure << endl;
	print_help( _c );
}

