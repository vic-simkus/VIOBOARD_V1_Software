#ifndef __COMMAND_LINE_PARMS__H__
#define __COMMAND_LINE_PARMS__H__


#include "lib/context.hpp"

#include <string>

class COMMAND_LINE_PARMS
{
	public:
		COMMAND_LINE_PARMS( size_t _argc, const char** _argv );

		void process( void );
		bool is_verbose_flag( void ) const;
		bool is_server_mode( void ) const;
		const string& get_address( void ) const;
		uint16_t get_port( void ) const;
		BBB_HVAC::SOCKET_TYPE get_socket_type( void ) const;
		const string& get_exe( void ) const;
		const string& get_log_file( void ) const;
	protected:

		void print_help( const string& _c );
		void print_cmd_error( const string& _c, const string& _failure );

		size_t argc;
		const char** argv;
	private:
		bool verbose_flag;
		bool server_mode;
		string address;
		string port_s;
		string log_file;
		uint16_t port_i;
		BBB_HVAC::SOCKET_TYPE st;
		string exe;


};


#endif