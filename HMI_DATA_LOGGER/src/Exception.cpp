#include "include/Exception.hpp"

using namespace HMI_DATA_LOGGER;

#include <sstream>
#include <iostream>

Exception::Exception( const Exception& _source ) : std::runtime_error( "HMI_DATA_LOGGER::Exception" )
{
	this->file = _source.file;
	this->line = _source.line;
	this->function = _source.function;
	this->message = _source.message;
	this->cause = _source.cause;
	this->std_cause = _source.std_cause;
	this->null = _source.null;
}
Exception::Exception() : std::runtime_error( "HMI_DATA_LOGGER::Exception(null)" )
{
	this->null = true;
}


Exception::Exception( const std::string& _file, size_t _line, const std::string& _function, const std::string& _message, const ExceptionPtr& _cause ) : std::runtime_error( "HMI_DATA_LOGGER::Exception" )
{
	//std::cout << "New exception @ " << _file << ": " << _message << ", cause: ";
	//_cause->_toString( std::cout );
	//std::cout << std::endl;

	this->file = _file;
	this->line = _line;
	this->function = _function;
	this->message = _message;
	this->cause = _cause;
	this->null = false;
}

Exception::Exception( const std::string& _file, size_t _line, const std::string& _function, const std::string& _message, const std::exception& _cause ) : std::runtime_error( "HMI_DATA_LOGGER::Exception" )
{
	//std::cout << "New exception @ " << _file << ":" << _message << ", cause: " << _cause.what() << std::endl;

	this->file = _file;
	this->line = _line;
	this->function = _function;
	this->message = _message;
	this->std_cause = _cause.what();
	this->null = false;
}

bool Exception::isNull( void ) const
{
	return this->null;
}
const std::string& Exception::getFile( void ) const
{
	return this->file;
}
const std::string& Exception::getFunction( void ) const
{
	return this->function;
}
size_t Exception::getLine( void ) const
{
	return this->line;
}
const ExceptionPtr& Exception::getCause( void ) const
{
	return this->cause;
}

std::string Exception::toString( void ) const
{
	std::stringstream str;
	this->toString( str );
	return str.str();
}

void Exception::toString( std::stringstream& _target ) const
{
	this->toString( _target, ExceptionPtr( new Exception( *this ) ) );
}
void Exception::toString( std::stringstream& _target, const ExceptionPtr& _what )
{
	if ( _what == nullptr || _what->isNull() )
	{
		// End of recursion.  Stop and return
		return;
	}

	if ( _what->cause != nullptr && !_what->cause->isNull() )
	{
		toString( _target, _what->cause );
	}
	else if ( _what->std_cause.length() > 0 )
	{
		_target << "[std::exception] " << _what->std_cause << std::endl;
	}

	_what->_toString( _target );
	_target << std::endl;
}

void Exception::_toString( std::ostream& _target ) const
{
	_target << this->file << ":" << this->line << " - " << this->message;
}
