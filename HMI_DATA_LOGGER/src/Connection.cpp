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

#include "include/Connection.hpp"
#include "include/Context.hpp"
#include "include/Exception.hpp"

using namespace HMI_DATA_LOGGER;

Connection::Connection( HMI_DATA_LOGGER::Context* _logger_context )
{
	INIT_LOGGER( "HMI_DATA_LOGGER::Connection" );

	this->logger_context = _logger_context;
	this->client_context = nullptr;

	return;
}
Connection::~Connection()
{
	if ( this->client_context )
	{
		this->client_context->disconnect();
	}

	this->logger_context->close_output_stream();
}

bool Connection::connect_to_logic_core( void )
{
	this->client_context = BBB_HVAC::CLIENT::CLIENT_CONTEXT::create_instance( this->logger_context->configuration.get_command_line_processor( )->get_socket_type(), this->logger_context->configuration.get_command_line_processor( )->get_address(), this->logger_context->configuration.get_command_line_processor( )->get_port() );

	try
	{
		this->client_context->connect();
	}
	catch ( const exception& e )
	{
		/*
			Hinky shit allert!!!

			If a connection is successfully launched and the client thread is launched, the client context instance cleanup is performed by the thread registry.
			IF the connection succeeds. If it doesn't the thread is not launched we end up with a memory leak and file descriptors that arent closed.
		*/
		delete this->client_context;
		this->client_context = nullptr;

		throw Exception( __FILE__, __LINE__, __FUNCTION__, "Failed to connect to logic core", e );
	}

	LOG_DEBUG( "Connected successfully to LOGIC_CORE" );

	this->get_item_names();

	return true;
}

std::list<std::string> Connection::get_item_names( void )
{
	if ( this->client_context == nullptr )
	{
		throw Exception( __FILE__, __LINE__, __FUNCTION__, "Not connected to remote" );
	}

	if ( this->logic_core_points.size() > 0 )
	{
		return this->logic_core_points;
	}

	BBB_HVAC::MESSAGE_PTR message;

	try
	{
		message = this->client_context->message_processor->create_read_logic_status( );
		message = this->client_context->send_message_and_wait( message );
	}
	catch ( const exception& e )
	{
		throw Exception( __FILE__, __LINE__, __FUNCTION__, "Failed to read from remote", e );
	}

	std::map<std::string, std::string> map;
	BBB_HVAC::MESSAGE::message_to_map( message, map );

	this->logic_core_points.clear();

	for ( auto map_iterator = map.begin(); map_iterator != map.end(); ++map_iterator )
	{
		this->logic_core_points.push_back( map_iterator->first );
	}

	return this->logic_core_points;

}