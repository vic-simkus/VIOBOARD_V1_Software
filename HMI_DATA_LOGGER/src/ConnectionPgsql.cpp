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

#include "include/ConnectionPgsql.hpp"
#include "include/Context.hpp"
#include "include/Exception.hpp"

#include <libpq-fe.h>

using namespace HMI_DATA_LOGGER;

ConnectionPgsql::ConnectionPgsql( HMI_DATA_LOGGER::Context* _ctx )  noexcept : Connection( _ctx )
{
	INIT_LOGGER( "HMI_DATA_LOGGER::ConnectionPgsql" );
	this->pg_connection = nullptr;
	return;
}

ConnectionPgsql::~ConnectionPgsql()
{
	this->clear_connection();
	return;
}
void ConnectionPgsql::clear_connection( void ) noexcept
{
	if ( this->pg_connection )
	{
		PQfinish( this->pg_connection );
	}

	this->pg_connection = nullptr;

	return;
}

void ConnectionPgsql::connect( void )
{
	if ( this->logger_context->configuration.pg_url.length() < 1 )
	{
		throw Exception( __FILE__, __LINE__, __FUNCTION__, "Command line error, must specify " + std::string( CFG_CMDP_PG_URL ) );
	}

	try
	{
		this->connect_to_logic_core() ;
	}
	catch ( const Exception& _e )
	{
		throw Exception( __FILE__, __LINE__, __FUNCTION__, "Failed to connect to logic core", ExceptionPtr( new Exception( _e ) ) );
	}

	string url = this->logger_context->configuration.get_command_line_value( CFG_CMDP_PG_URL );

	this->pg_connection = PQconnectdb( url.data() );

	if(this->pg_connection == nullptr)
	{
		throw Exception(__FILE__,__LINE__,__FUNCTION__,"PQConnectdb returned a NULL");
	}

	if ( PQstatus( this->pg_connection ) != CONNECTION_OK )
	{
		LOG_ERROR("Failed to connect to database using [" + url + "]: "  + std::string( PQerrorMessage( this->pg_connection ) ));
		this->clear_connection();
		throw Exception( __FILE__, __LINE__, __FUNCTION__, "Failed to connect to database.  Check error log." );
	}

	LOG_DEBUG( "Connected to the database." );

	return;
}

void ConnectionPgsql::disconnect( void )
{
	this->clear_connection();
}

void ConnectionPgsql::read_status( void )
{
	if ( this->client_context == nullptr )
	{
		throw Exception( __FILE__, __LINE__, __FUNCTION__, "Client context is null" );
	}

	BBB_HVAC::MESSAGE_PTR message;

	try
	{
		message = this->client_context->message_processor->create_read_logic_status( );
		message = this->client_context->send_message_and_wait( message );
	}
	catch ( const exception& e )
	{
		throw Exception( __FILE__, __LINE__, __FUNCTION__, "Exception in remote comms" , e );
	}
	catch ( ... )
	{
		throw Exception( __FILE__, __LINE__, __FUNCTION__, "Unspecified exception." );

	}

	std::map<std::string, std::string> map;
	std::list<std::string> names;
	std::list<std::string> values;

	//LOG_DEBUG( message->to_string() );
	BBB_HVAC::MESSAGE::message_to_map( message, map );

	for ( auto i = map.begin(); i != map.end(); ++i )
	{
		names.push_back( i->first );
		values.push_back( i->second );
	}

	string sql = "INSERT INTO data. home_data (" + join_list( names, ',' ) + ") VALUES (" + join_list( values, ',' ) + ")";


	PqResultGuard res(	this->execute_sql( sql ) );

	if ( res.is_null() )
	{
		throw Exception( __FILE__, __LINE__, __FUNCTION__, "NULL result" );
	}

}

bool ConnectionPgsql::test_connection() noexcept
{
	LOG_DEBUG( "Testing DB connection." );


	try
	{
		this->connect();
	}
	catch ( const Exception& _e )
	{
		LOG_ERROR( "Connection test failed: " + _e.toString() );
		return false;
	}

	PqResultGuard res( this->execute_sql( "select version();" ) );

	if ( !res.is_null() )
	{
		if ( PQntuples( res.get() ) != 1 )
		{
			LOG_ERROR( "Received an unexpected number of tuples: " + num_to_str( PQntuples( res.get() ) ) );
			return false;
		}

		if ( PQnfields( res.get() ) != 1 )
		{
			LOG_ERROR( "Received an unexpected number of fields: " + num_to_str( PQnfields( res.get() ) ) );
			return false;
		}

		LOG_INFO( "Server response: " + std::string( PQgetvalue( res.get(), 0, 0 ) ) );


		return true;
	}
	else
	{
		LOG_ERROR( "Failed to execute test SQL query." );
		return false;
	}
}

PGresult* ConnectionPgsql::execute_sql( const std::string& _sql ) noexcept
{
	if ( _sql.length() < 1 )
	{
		return nullptr;
	}

	PqResultGuard res( PQexec( this->pg_connection, _sql.data() ) );

	if ( PQresultStatus( res.get() ) != PGRES_TUPLES_OK && PQresultStatus( res.get() ) != PGRES_COMMAND_OK )
	{
		LOG_ERROR( "Failed to execute SQL [" + _sql + "]: " + std::string( PQerrorMessage( this->pg_connection ) ) );

		return nullptr;
	}
	else
	{
		return res.claim();
	}
}

PqResultGuard::PqResultGuard( PGresult* _ptr )
{
	this->pg_result = _ptr;
}
PqResultGuard::~PqResultGuard()
{
	if ( this->pg_result != nullptr )
	{
		PQclear( this->pg_result );
	}

	this->pg_result = nullptr;

	return;
}

bool PqResultGuard::is_null( void ) const
{
	return ( this->pg_result == nullptr );
}
PGresult* PqResultGuard::get( void )
{
	return this->pg_result;
}
PGresult* PqResultGuard::claim( void )
{
	PGresult* ret = this->pg_result;
	this->pg_result = nullptr;

	return ret;
}
