#ifndef __COMMAND_LINE_PARMS__H__
#define __COMMAND_LINE_PARMS__H__


#include "lib/context.hpp"

#include <map>
#include <list>

namespace BBB_HVAC
{
	class COMMAND_LINE_PARMS
	{
		public:
			typedef std::map<std::string, std::string> EX_PARAM_LIST;
			COMMAND_LINE_PARMS();
			COMMAND_LINE_PARMS( size_t _argc, const char** _argv );
			COMMAND_LINE_PARMS( size_t _argc, const char** _argv, const EX_PARAM_LIST& _ex_parm_list );

			void process( void );
			bool is_verbose_flag( void ) const;
			bool is_server_mode( void ) const;
			const string& get_address( void ) const;
			uint16_t get_port( void ) const;
			BBB_HVAC::SOCKET_TYPE get_socket_type( void ) const;
			const string& get_exe( void ) const;
			const string& get_log_file( void ) const;
			const string& get_config_file( void ) const;

			EX_PARAM_LIST ex_parm_values;

		protected:
			void init( size_t _argc, const char** _argv );
			void print_help( const string& _c );
			void print_cmd_error( const string& _c, const string& _failure );
			bool process_ex_params( const string& _p );
			size_t argc;
			const char** argv;

		private:
			EX_PARAM_LIST ex_parm_list;
			std::list<string> ex_parm_names;
			bool verbose_flag;
			bool server_mode;
			string address;
			string port_s;
			string log_file;
			string config_file;
			uint16_t port_i;
			BBB_HVAC::SOCKET_TYPE st;
			string exe;
	};
}
#endif