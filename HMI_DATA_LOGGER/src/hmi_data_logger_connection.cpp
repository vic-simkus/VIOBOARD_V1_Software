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

#include "include/hmi_data_logger_connection.hpp"
#include "include/hmi_data_logger_context.hpp"

#include <vector>

using namespace HMI_DATA_LOGGER;

bool HMI_DATA_LOGGER_CONNECTION::read_status( void )
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
	catch ( const exception& e )
	{
		LOG_ERROR( "Failed to read from remote: " + std::string( e.what() ) );
		return false;
	}

	std::map<std::string, std::string> map;
	BBB_HVAC::MESSAGE::message_to_map( message, map );

	if ( this->logger_context->get_new_file_flag() )
	{
		this->logic_core_points.clear();

		for ( auto map_iterator = map.begin(); map_iterator != map.end(); ++map_iterator )
		{
			this->logic_core_points.push_back( map_iterator->first );
		}

		this->logger_context->get_output_stream() << join_vector( this->logic_core_points, ',' ) << std::endl;
		this->logger_context->reset_new_file_flag();
	}

	std::vector<std::string> output_vector;
	output_vector.push_back( get_iso_date_time() );

	for ( auto vector_iterator = this->logic_core_points.begin(); vector_iterator != this->logic_core_points.end(); ++vector_iterator )
	{
		output_vector.push_back( map[*vector_iterator] );
	}

	this->logger_context->get_output_stream() << join_vector( output_vector, ',' ) << std::endl;
	return true;
}

HMI_DATA_LOGGER_CONNECTION::HMI_DATA_LOGGER_CONNECTION( HMI_DATA_LOGGER::HMI_DATA_LOGGER_CONTEXT* _logger_context )
{
	this->logger_context = _logger_context;
	INIT_LOGGER( "HMI_DATA_LOGGER::HMIC_DATA_LOGGER_CONNECTION" );
	LOG_DEBUG( "Instantiating." );
	this->client_context = BBB_HVAC::CLIENT::CLIENT_CONTEXT::create_instance( this->logger_context->configuration.command_line_parms->get_socket_type(), this->logger_context->configuration.command_line_parms->get_address(), this->logger_context->configuration.command_line_parms->get_port() );
	this->opened_output = false;
	return;
}

bool HMI_DATA_LOGGER_CONNECTION::connect( void )
{
	try
	{
		this->client_context->connect();
	}
	catch ( const BBB_HVAC::EXCEPTIONS::CONNECTION_ERROR& e )
	{
		LOG_ERROR( "Failed to connect to logic core: " + std::string( e.what() ) );
		return false;
	}

	return true;
}

bool HMI_DATA_LOGGER_CONNECTION::disconnect( void )
{
	this->client_context->disconnect();
	return true;
}

HMI_DATA_LOGGER_CONNECTION::~HMI_DATA_LOGGER_CONNECTION()
{
	if ( this->client_context )
	{
		this->client_context->disconnect();
	}

	//delete this->client_context;
	this->logger_context->close_output_stream();
}

