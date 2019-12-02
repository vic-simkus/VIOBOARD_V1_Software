#include "MESSAGE_BUS.h"

DEF_LOGGER_STAT( "MESSAGE_BUS::MESSAGE" );

MESSAGE_BUS::MESSAGE::MESSAGE( const QString& _board_id, COMMANDS _command, const QVariant& _payload )
{
	this->board_id = _board_id;
	this->command = _command;
	this->payload = _payload;
}

MESSAGE_BUS::MESSAGE::MESSAGE( const QString& _board_id, COMMANDS _command )
{
	this->board_id = _board_id;
	this->command = _command;
}

MESSAGE_BUS::MESSAGE::MESSAGE( COMMANDS _command, const QVariant& _payload )
{
	this->command = _command;
	this->payload = _payload;
}

MESSAGE_BUS::MESSAGE::MESSAGE( COMMANDS _command )
{
	this->command = _command;
}

MESSAGE_BUS::MESSAGE MESSAGE_BUS::MESSAGE::create_message_set_cal_vals( const QString& _board_id, const QVector<uint16_t>& _cal_val_l1, const QVector<uint16_t>& _cal_val_l2 )
{

	QList<QVariant> parms;

	QList<QVariant> l1_vals;
	QList<QVariant> l2_vals;

	for ( auto i = _cal_val_l1.begin(); i != _cal_val_l1.end(); ++i )
	{
		//LOG_DEBUG_STAT( "Appending L1 cal value: " + num_to_str( *i ) );

		l1_vals.append( QVariant( *i ) );
	}

	for ( auto i = _cal_val_l2.begin(); i != _cal_val_l2.end(); ++i )
	{
		//LOG_DEBUG_STAT( "Appending L2 cal value: " + num_to_str( *i ) );
		l2_vals.append( QVariant( *i ) );
	}

	//LOG_DEBUG_STAT( "L1 variant size: " + num_to_str( l1_vals.count() ) );
	//LOG_DEBUG_STAT( "L2 variant size: " + num_to_str( l2_vals.count() ) );

	parms.append( QVariant( l1_vals ) );
	parms.append( QVariant( l2_vals ) );

	return MESSAGE( _board_id, COMMANDS::SET_CAL_VALS, QVariant( parms ) );
}

MESSAGE_BUS::MESSAGE MESSAGE_BUS::MESSAGE::create_message_set_do_status( const QString& _board_id, const uint8_t _do_status )
{
	return MESSAGE( _board_id, COMMANDS::SET_DO, QVariant( _do_status ) );
}

MESSAGE_BUS::MESSAGE MESSAGE_BUS::MESSAGE::create_message_set_pmic_status( const QString& _board_id, const uint8_t _pmic_status )
{
	return MESSAGE( _board_id, COMMANDS::SET_PMIC, QVariant( _pmic_status ) );
}
