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

#include "include/ConnectionFile.hpp"
#include "include/Context.hpp"
#include "include/Exception.hpp"

#include <vector>

using namespace HMI_DATA_LOGGER;

ConnectionFile::ConnectionFile( HMI_DATA_LOGGER::Context* _ctx ) : Connection( _ctx )
{
	INIT_LOGGER( "HMI_DATA_LOGGER::ConnectionFile" );

	this->opened_output = false;

	return;
}

ConnectionFile::~ConnectionFile()
{
	return;
}

void ConnectionFile::read_status( void )
{
	if ( !this->opened_output )
	{
		this->logger_context->open_output_stream();
		this->opened_output = true;
	}

	BBB_HVAC::MESSAGE_PTR message;

	try
	{
		message = this->client_context->message_processor->create_read_logic_status( );
		message = this->client_context->send_message_and_wait( message );
	}
	catch ( const exception& _e )
	{
		throw Exception( __FILE__, __LINE__, __FUNCTION__, "Failed to read from LOGIC_CORE.", _e );
	}

	std::map<std::string, std::string> map;
	BBB_HVAC::MESSAGE::message_to_map( message, map );

	if ( this->logger_context->get_new_file_flag() )
	{
		this->logger_context->get_output_stream() << join_list( this->logic_core_points, ',' ) << std::endl;
		this->logger_context->reset_new_file_flag();
	}

	std::vector<std::string> output_vector;
	output_vector.push_back( get_iso_date_time() );

	for ( auto vector_iterator = this->logic_core_points.begin(); vector_iterator != this->logic_core_points.end(); ++vector_iterator )
	{
		output_vector.push_back( map[*vector_iterator] );
	}

	this->logger_context->get_output_stream() << join_vector( output_vector, ',' ) << std::endl;
}


void ConnectionFile::connect( void )
{
	try
	{
		this->connect_to_logic_core();
	}
	catch ( const Exception& _e )
	{
		throw Exception( __FILE__, __LINE__, __FUNCTION__, "Failed to connect to LOGIC_CORE.", ExceptionPtr( new Exception( _e ) ) );
	}
}

void ConnectionFile::disconnect( void )
{
	this->client_context->disconnect();
}
