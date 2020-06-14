DROP TABLE IF EXISTS data.home_data;

CREATE TABLE data.home_data
(
	row_id SERIAL NOT NULL PRIMARY KEY,
	time_stamp TIMESTAMP NOT NULL UNIQUE DEFAULT (now()),
	ac_compressor BOOL NOT NULL,
	ahu_fan BOOL NOT NULL,
	ahu_heater BOOL NOT NULL,
	ahu_return_temp REAL NOT NULL,
	ahu_supply_temp REAL NOT NULL,
	attic_temp REAL NOT NULL,
	space_1_rh REAL NOT NULL,
	space_1_temp REAL NOT NULL,
	outdoor_temp REAL NOT NULL DEFAULT '-459.67',
	outdoor_rh REAL NOT NULL DEFAULT '-1'
);