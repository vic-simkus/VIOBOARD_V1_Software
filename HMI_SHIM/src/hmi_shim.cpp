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


#include "lib/bbb_hvac.hpp"
#include "lib/message_lib.hpp"
#include "lib/message_types.hpp"
#include "lib/message_processor.hpp"
#include "lib/globals.hpp"
#include "lib/hvac_types.hpp"
#include "lib/serial_io_types.hpp"
#include "lib/config.hpp"

#include "lib/threads/thread_registry.hpp"

#include <iostream>
#include <vector>
#include <string>

#include <unistd.h>

using namespace std;
using namespace BBB_HVAC;
using namespace BBB_HVAC::CLIENT;

size_t calc_ai_offset( size_t x, size_t y )
{
	return ( GC_IO_AI_COUNT * y ) + x;
}

void client_main( CLIENT_CONTEXT* ctx ) throw( exception )
{
	CAL_VALUE_ARRAY vals;
	vals.push_back( 0 );
	vals.push_back( 1 );
	vals.push_back( 2 );
	vals.push_back( 3 );
	vals.push_back( 4 );
	vals.push_back( 5 );
	vals.push_back( 6 );
	vals.push_back( 7 );
	size_t idx = 0;

	while( 1 )
	{
		MESSAGE_PTR m = ctx->message_processor->create_set_l1_cal_vals( "BOARD1", vals );
		ctx->send_message( m );
		cout << "[" << idx << "] -- " << m->to_string() << endl;
		idx += 1;
		//usleep(100);
		//sleep(1);
	}

	cout << endl;
	cout << endl;
	cout << endl;
}

int main( void )
{
	GLOBALS::configure_logging( LOGGING::ENUM_LOG_LEVEL::DEBUG );
	GLOBALS::configure_signals();
	CLIENT_CONTEXT* ctx = CLIENT_CONTEXT::create_instance();

	try
	{
		ctx->connect();
	}
	catch( const exception& _e )
	{
		cout << string( "Failed to connect to server: " ) + _e.what();
		exit( -1 );
	}

	try
	{
		client_main( ctx );
	}
	catch( const exception& _e )
	{
		cerr << "Caught exception in client main: " + string( _e.what() ) << endl;
	}

	THREAD_REGISTRY::stop_all();
	THREAD_REGISTRY::init_cleanup();
	return 0;
}
