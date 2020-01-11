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


#ifndef SRC_INCLUDE_LIB_HVAC_TYPES_HPP_
#define SRC_INCLUDE_LIB_HVAC_TYPES_HPP_

#include <vector>
#include <memory>
#include <map>
#include <ostream>
#include <utility>

using namespace std;

/**
 * \file Various HVAC control system types
 */
namespace BBB_HVAC
{
	/**
	Type of analog input
	*/

	enum class AI_TYPE : unsigned int
	{
		INVALID = 0,
		CL_420,
		ICTD
	};

	/**
	 * Message direction enum
	 */
	enum class ENUM_MESSAGE_DIRECTION : unsigned int
	{
		IN = 0, /// In
		OUT		/// Out
	} ;

	/**
	 * Message processor callback result
	 */
	enum class ENUM_MESSAGE_CALLBACK_RESULT : unsigned int
	{
		PROCESSED = 0,		/// Message was processed by callback.  Message processing will stop.
		IGNORED				/// Message was ignored by the callback.  Message processing will continue.
	} ;

	class MESSAGE;

	/**
	 * Manage message pointer type
	 */
	typedef std::shared_ptr<MESSAGE> MESSAGE_PTR;

	/**
	 * Defines behavior in a situation where an attemp tto add a message has been made, but the message queue is full.
	 */
	enum class ENUM_APPEND_MODE : unsigned int
	{

		LOSE_OVERFLOW = 0,	/// Delete the oldest message.
		ERROR_OVERFLOW		/// Throw an exception
	} ;

	/**
	 * Internal message queue implementation typedef.
	 */
	typedef std::vector<MESSAGE_PTR> MESSAGE_VECTOR;

	/**
	 * Enum of all supported message types.
	 * In order to add a new message type to the system:
	 * 	-# Add an enum value to this type.
	 * 	-# Add string representation to static 'std::string __message_type_list[]' in message_types.cpp.
	 * 	-# Add response processing of the message to the code.  BBB_HVAC::SERVER::HS_CLIENT_CONTEXT::process_message in context.cpp is a good start.
	 * 	-# If wanted, add sanity checking to BBB_HVAC::MESSAGE_PROCESSOR::parse_message(const std::string&).
	 * 	-# If wanted, add creation method to BBB_HVAC::MESSAGE_PROCESSOR.
	 *
	 */
	enum class ENUM_MESSAGE_TYPE : unsigned int
	{
		INVALID = 0, 				/// Invalid message
		PING,						/// Ping message.  Used to invoke a 'pong' response.  Intended to be used as a check of the connection status
		PONG,						/// Response to ping.  Intended to be used as a check of the connection status
		HELLO,						/// Opening message during connection establishment.
		READ_STATUS,				/// Requests the status of all inputs and outputs.
		READ_STATUS_RAW_ANALOG,		/// Requests the raw analog DAC values from the IO board.
		SET_STATUS, 				/// Requests for outputs to be set to specified states.
		SET_PMIC_STATUS,			///	Requests for the DO and AI PMICs to be set to specified states.
		GET_LABELS, 				/// Requests the map between human readable labels and input/output ports.
		SET_POINT,					/// Sets an adjustable point in the logic core
		ERROR,						/// Error response.
		GET_L1_CAL_VALS,			/// Get L1 calibration values
		GET_L2_CAL_VALS,			///	Get L2 calibration values
		SET_L1_CAL_VALS,			/// Set L1 calibration values
		SET_L2_CAL_VALS,			/// Set L2 calibration values
		GET_BOOT_COUNT,				/// Get board boot count
		READ_LOGIC_STATUS,			/// Gets the point statuses from the logic thread.  This will include all calculated analog values as the logic thread sees it.
		FORCE_AI_VALUE,				/// Forces an input value.
		UNFORCE_AI_VALUE,			/// Unfores an input value.
		__MSG_END__					/// Terminator of the enum.  Used in iterating through the enum values.
	} ;

	inline std::ostream& operator<< ( std::ostream& os, ENUM_MESSAGE_TYPE _v )
	{
		return os << static_cast < unsigned int >( _v );
	}

	namespace LOGGING
	{
		class LOGGER;


		typedef std::shared_ptr<BBB_HVAC::LOGGING::LOGGER > LOGGER_PTR;
	}

	class __MESSAGE_TYPE;
	/**
	 * Typedef of the std::shared_ptr wrapper around __MESSAGE_TYPE.
	 * This type will be passed around rather than the raw instances __MESSAGE_TYPE.
	 */
	typedef std::shared_ptr<__MESSAGE_TYPE> MESSAGE_TYPE;


	/**
	 * An enum to message type map type definition.
	 */
	typedef std::map<ENUM_MESSAGE_TYPE, MESSAGE_TYPE> MAP_TYPE_TO_LABEL;

	/**
	 * A label to message type map type definition.
	 */
	typedef std::map<std::string, MESSAGE_TYPE> MAP_LABEL_TO_TYPE;

	typedef std::vector<uint16_t> CAL_VALUE_ARRAY;
}

#endif /* SRC_INCLUDE_LIB_HVAC_TYPES_HPP_ */
