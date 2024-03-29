#include "lib/command_line_parms.h"

#include <iostream>
using namespace BBB_HVAC;

// For getaddrinfo and associated
#include <netdb.h>

// For exception
#include <exception>
using std::runtime_error;

COMMAND_LINE_PARMS::COMMAND_LINE_PARMS()
{
	this->init( 0, nullptr );
	return;
}

COMMAND_LINE_PARMS::COMMAND_LINE_PARMS( size_t _argc, const char** _argv )
{
	this->init( _argc, _argv );
	return;
}

COMMAND_LINE_PARMS::COMMAND_LINE_PARMS( size_t _argc, const char** _argv, const EX_PARAM_LIST& _ex_parm_list )
{
	this->init( _argc, _argv );

	for ( auto i = _ex_parm_list.begin(); i != _ex_parm_list.end(); ++i )
	{
		this->ex_parm_names.push_back( i->first );
	}

	this->ex_parm_list = _ex_parm_list;

	return;
}

void COMMAND_LINE_PARMS::init( size_t _argc, const char** _argv )
{
	this->argc = _argc;
	this->argv = _argv;

	this->verbose_flag = false;
	this->server_mode = false;

	address = "";
	port_s = "";

	port_i =  GC_DEFAULT_TCPIP_PORT;

	this->st = BBB_HVAC::SOCKET_TYPE::NONE;

	if ( _argv != nullptr )
	{
		this->exe = string( argv[0] );
	}

	// Log file is set in COMMAND_LINE_PARMS::process
	//this->log_file = "/var/log/BBB_HVAC.log";
	this->config_file = "configuration.dev.cfg";

	/*
	Default for address depends on the socket type.
	It's set in COMMAND_LINE_PARMS::process
	*/

	return;
}

bool COMMAND_LINE_PARMS::process_ex_params( const string& _p )
{
	for ( auto i = this->ex_parm_names.begin(); i != this->ex_parm_names.end(); ++i )
	{
		if ( ( *i ) == _p )
		{
			return true;
		}
	}

	return false;
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
		else if ( p == "-h" || p == "-?" )
		{
			print_help( exe );
			exit( 0 );
		}
		else if ( p == "-v" )
		{
			verbose_flag = true;

			cout << "Verbose flag enabled.  Will produce extra output." << endl;
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
		else if ( p == "-c" )
		{
			if ( i == ( argc - 1 ) )
			{
				print_cmd_error( exe, "No configuration file specified for the -c parameter." );
			}
			else
			{
				config_file = string( argv[i + 1] );
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
				if ( st == BBB_HVAC::SOCKET_TYPE::TCPIP )
				{

					address = string( argv[i + 1] );

					struct addrinfo* result;
					struct addrinfo hints;
					memset( &hints, 0, sizeof( hints ) );

					hints.ai_family = AF_UNSPEC;
					hints.ai_flags = AI_CANONNAME | AI_V4MAPPED | AI_ADDRCONFIG;

					int rc = getaddrinfo( address.data(), nullptr, &hints, &result );

					if ( rc != 0 )
					{
						string e = "Failed to resolve address [" + address + "]: " + string( gai_strerror( rc ) );
						throw ( runtime_error( e.data() ) );
					}

					char hbuf[NI_MAXHOST];

					rc = getnameinfo( result->ai_addr, sizeof( sockaddr ), hbuf, sizeof( hbuf ), nullptr, 0, NI_NUMERICHOST );

					if ( rc != 0 )
					{
						string e = "Failed to convert IP address to string: " + string( gai_strerror( rc ) );
						throw ( runtime_error( e.data() ) );
					}


					if ( verbose_flag )
					{
						cout << "Resolved address: " << string( result->ai_canonname ) << ", " << string( hbuf )  <<  endl;
					}

					freeaddrinfo( result );

					address = string( hbuf );
				}

				i += 1;
			}
		}
		else if ( p == "-s" )
		{
			this->server_mode = true;
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
			if ( !this->process_ex_params( p ) )
			{
				print_cmd_error( exe, "Unrecognized command line parameter: " + p );
			}
			else
			{
				if ( i == ( argc - 1 ) )
				{
					print_cmd_error( exe, "No parameter specified for: " + p );
				}
				else
				{
					//cout << "Setting: " << p << " to: " << std::string( argv[i + 1] ) << endl;
					this->ex_parm_values[p] = string( argv[i + 1] );
					i += 1;
				}
			}
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

	if ( !server_mode )
	{
		// We're NOT in server mode.

		if ( this->log_file.length() < 1 )
		{
			// log file has not been explicitly specified.
			// If we're not in server mode we default to STDOUT as log destination
			//
			if ( verbose_flag )
			{
				cout << "    Server mode is false and a log file has not been specified.  Defaulting log output to STDOUT" << endl;
			}

			this->log_file = "-";
		}
	}
	else
	{
		if ( this->log_file.length() < 1 )
		{
			// Log file has not been specified.
			this->log_file = "/var/log/BBB_HVAC.log";
		}
	}

	if ( verbose_flag )
	{
		cout << "Finished parsing command line." << endl;
	}

	return;

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

const string& COMMAND_LINE_PARMS::get_config_file( void ) const
{
	return this->config_file;
}

void COMMAND_LINE_PARMS::print_help( const string& _c )
{
	cout << "Minimum required parameters: " << endl;
	cout << _c << " {-d|i}" << endl;
	cout << "Where: " << endl;
	cout << "\t-d - Listen on domain socket (mutually exclusive with -i)" << endl;
	cout << "\t-i - Listen on TCP/IP socket (mutually exclusive with -d)" << endl;
	cout << "\t-a <address> - Address to listen on.  File name (optional) if -d is specified.  Interface to bind to if -i is specified." << endl;
	cout << "\t  For -i, the interface should be specified in x.x.x.x notation. Default for -i is 127.0.0.1." << endl;
	cout << "\t  Default for -d is " << endl;
	cout << "\t-p <port> - Port to listen to.  Relevant only if -i is specified.  Defaults to 6666" << endl;
	cout << "\t-l <log> - Log file.  Defaults to /var/log/BBB_HVAC.log.  Specify - to log to stdout." << endl;
	cout << "\t-s - Server mode.  If not specified application runs in console." << endl;
	cout << "\t-v - Verbose mode.  Produces extra debugging information to the console.  Verbose flag should be first in the parameter list." << endl;
	cout << "\t-h - This help" << endl;
	cout << "\t-c - Configuration file.  Defaults to configuration.dev.cfg" << endl;
	cout << "NOTE: Parameters are position sensitive.  -v should be first in order to capture all possible command line processing output.  -a should be specified AFTER -i." << endl;

	if ( this->ex_parm_list.size() > 0 )
	{
		cout << "\n\t***********" << endl << endl;

		for ( auto i = this->ex_parm_list.begin(); i != this->ex_parm_list.end(); i++ )
		{
			cout << "\t" << i->first <<  " " << i->second << endl;
		}
	}

	exit( -1 );

	return;
}

void COMMAND_LINE_PARMS::print_cmd_error( const string& _c, const string& _failure )
{
	cerr << "Command line error: " << endl;
	cerr << _failure << endl;
	print_help( _c );

	return;
}

