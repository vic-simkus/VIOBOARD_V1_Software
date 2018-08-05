/*
* This file is part of the software stack for Vic's IO board and its
* associated projects.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* Copyright 2016,2017,2018 Vidas Simkus (vic.simkus@gmail.com)
*/


#ifndef CONFIGURATOR_TYPES_HPP
#define CONFIGURATOR_TYPES_HPP

#include <vector>
#include <string>

namespace BBB_HVAC
{
	/**
	 * Types of configuration elements
	 */
	enum class ENUM_CONFIG_TYPES : unsigned int
	{
		INVALID = 0, 	/// Invalid value.
		DO,				/// Digital output.  A digital output on an IO board.
		AI,				/// Analog input.  An analog input on an IO board.
		SP,				/// Set-point.  A value that the system will try to achieve.  Meaning of setpoint is based on context.
		BOARD,			/// IO Board entry.  Defines an IO board and its communication port.
		MAP				/// Mapping entry.
	};

	/**
	CT (Configuration Type) BOARD field indexes.  Every BOARD configuration entry needs to have the following fields in the specified order..
	*/
	enum class ENUM_CT_BOARD_IDX : unsigned int
	{
		TAG,			/// Tag/ID of the board.
		DEVICE,			/// Linux serial device name.
		OPTIONS,		/// Options.
	};

	/**
	CT (Configuration Type) DO field indexes.  Every DO (digital output) configuration entry needs to have the following fields in the specified order.
	*/
	enum class ENUM_CT_POINT_IDX : unsigned int
	{
		BOARD_TAG,		/// Parent board tag/ID.
		INDEX,			/// Board input index.
		DESCRIPTION		/// Description of the output.
	};

	/**
	CT (Configuration Type) SP field indexes.  Every SP (set point) configuration entry needs to  have the following fields in the specified order.
	*/
	enum class ENUM_CT_SP_IDX : unsigned int
	{
		DESCRIPTION,	/// Set point description.
		VALUE			/// Set point value.
	};


	/**
	 * Type for a list of configuration parts.
	 */
	typedef std::vector<std::string> CONFIG_PARTS_TYPE;

	/**
	 * Forward declaration
	 */
	class CONFIG_ENTRY;

	/**
	 * Type for list of configuration entries
	 */
	typedef std::vector<CONFIG_ENTRY> CONFIG_ENTRY_LIST_TYPE;

	/**
	 * Type for list of configuration type indexes
	 */
	typedef std::vector<size_t> CONFIG_TYPE_INDEX_TYPE;

	/**
	Forward declaration
	*/
	class BOARD_POINT;

	typedef std::vector<BOARD_POINT> BOARD_POINT_VECTOR;

	/**
	Forward declration
	*/
	class SET_POINT;

	typedef std::map<std::string, SET_POINT> SET_POINT_MAP;

}

#endif