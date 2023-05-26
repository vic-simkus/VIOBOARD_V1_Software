#!/usr/bin/env bash
export LD_LIBRARY_PATH=../HVAC_LIB/bin
./bin/HMI_DATA_LOGGER -l - -d --mode PGSQL --pg_url postgresql://bbb_hvac@ECHELON/bbb_hvac 
