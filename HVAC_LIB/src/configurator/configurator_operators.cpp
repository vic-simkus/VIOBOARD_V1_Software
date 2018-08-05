#include "lib/configurator.hpp"

#include <ostream>

using namespace BBB_HVAC;

namespace BBB_HVAC
{

	std::ostream& operator<< ( std::ostream& os, ENUM_CONFIG_TYPES _v )
	{
		return os << static_cast < unsigned int >( _v );
	}

	bool operator==( const BOARD_POINT& _l, const BOARD_POINT& _r )
	{
		if ( _l.type == _r.type && _l.board_tag == _r.board_tag && _l.point_id == _r.point_id )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}