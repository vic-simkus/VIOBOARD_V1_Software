#ifndef __COMMAND_LINE_PARMS__H__
#define __COMMAND_LINE_PARMS__H__


#include "lib/context.hpp"

#include <map>
#include <list>

namespace BBB_HVAC
{
	/**
	 * Command line processor
	 */
	class COMMAND_LINE_PARMS
	{
		public:
			/**
			 * Typedef for extra parameters.  Key is the parameter name i.e. '--parameter'.  Value is the description of the parameter that will be outputted when help is requested.
			 */
			typedef std::map<std::string, std::string> EX_PARAM_LIST;

			/**
			 * Contstructor
			 */
			COMMAND_LINE_PARMS();

			/**
			 * Constructor with the main() parameters
			 * No processing is done at time of initialization.
			 */
			COMMAND_LINE_PARMS( size_t _argc, const char** _argv );

			/**
			 * Constructor with the main parameters and extra parameters
			 * No processing is done at time of initialization.
			 */
			COMMAND_LINE_PARMS( size_t _argc, const char** _argv, const EX_PARAM_LIST& _ex_parm_list );

			/**
			 * Processes the command line parameters specified in the constructors.
			 * Method calls exit() if  the -h or --help command line is supplied or if there's a command line error.
			 */
			void process( void );

			/**
			 *  Returns TRUE if the verbose flag was set
			 */
			bool is_verbose_flag( void ) const;

			/**
			 *  Returns TRUE if the program should be run in server or daemon mode.
			 *  This option is either redundant or hokey depending on your POV.  All future daemonizations are to be done using the platform's init system
			 */
			bool is_server_mode( void ) const;

			/**
			 * Returns specified address.  The returned address is usable in the context classes.  In other words, it's a valid IP address.
			 */
			const string& get_address( void ) const;

			/**
			 * Returns the specified port.  The port is whatever the user specified at the command line or a default port if one was not specified.
			 */
			uint16_t get_port( void ) const;

			/**
			 * Are we operating on a network socket or a local file.
			 */
			BBB_HVAC::SOCKET_TYPE get_socket_type( void ) const;

			/**
			 * Returns the executable name.
			 */
			const string& get_exe( void ) const;

			/**
			 * Returns the log file.
			 */
			const string& get_log_file( void ) const;

			/**
			 * Returns the configuration file.
			 */
			const string& get_config_file( void ) const;

			/**
			 * Extra configuration parameter values.  The key is the parameter name.
			 */
			EX_PARAM_LIST ex_parm_values;

		protected:

			/**
			 * Initializes the instance.
			 */
			void init( size_t _argc, const char** _argv );

			/**
			 * Dumps command line parameter help to cout.  This method does not return.
			 * \param _c Executable name
			 */
			void print_help( const string& _c );

			/**
			 * Dumps command line error to cerr.  This method does not return.
			 * \_c Executable name
			 * \_failre Failure message
			 */
			void print_cmd_error( const string& _c, const string& _failure );

			/**
			 * Returns TRUE if _p is in ex_parm_names.  False otherwise.
			 */
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