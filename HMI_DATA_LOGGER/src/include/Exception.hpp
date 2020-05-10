#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <exception>
#include <memory>
#include <string>
#include <sstream>

namespace HMI_DATA_LOGGER
{
	class Exception;

	typedef std::shared_ptr<Exception> ExceptionPtr;

	class Exception : public std::runtime_error
	{
		public:
			Exception( const Exception& _source );
			Exception();
			Exception( const std::string& _file, size_t _line, const std::string& _function, const std::string& _message, const ExceptionPtr& _cause = ExceptionPtr() );
			Exception( const std::string& _file, size_t _line, const std::string& _function, const std::string& _message, const std::exception& _cause );

			bool isNull( void ) const;
			const std::string& getFile( void ) const;
			const std::string& getFunction( void ) const;
			size_t getLine( void ) const;
			const ExceptionPtr& getCause( void ) const;
			std::string toString( void ) const;
			void toString( std::stringstream& _target ) const;
			static void toString( std::stringstream& _target, const ExceptionPtr& _what );
		protected:
			bool null;
			std::string file;
			std::string function;
			std::string message;
			size_t line;
			ExceptionPtr cause;
			std::string std_cause;

			void _toString( std::ostream& _target ) const;

		private:

	};
}

#endif
