#ifndef __LOG_CONFIGURATOR_H__
#define __LOG_CONFIGURATOR_H__

#include "lib/threads/tprotect_base.hpp"

namespace BBB_HVAC
{
	namespace LOGGING
	{
		class LOG_CONFIGURATOR : public TPROTECT_BASE
		{
			public:
				LOG_CONFIGURATOR( int _fd, ENUM_LOG_LEVEL _level );
				~LOG_CONFIGURATOR();

				ENUM_LOG_LEVEL get_level( void ) const;

				static LOG_CONFIGURATOR* get_root_configurator( void );
				static void destroy_root_configurator( void );

				void log( const string& _log_name, const ENUM_LOG_LEVEL& _level, const string& _msg, const string& _file, int _line, const string& _function );

				bool is_had_error( void ) const;
			protected:
				ENUM_LOG_LEVEL level;
				static LOG_CONFIGURATOR* root_configurator;
				int fd;
				int had_error;

		};
	}
}

#endif