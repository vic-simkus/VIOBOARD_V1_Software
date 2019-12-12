#!/usr/bin/env bash

echo "Starting LOGIC_CORE"

export LD_LIBRARY_PATH="../HVAC_LIB/bin:$LD_LIBRARY_PATH"

./bin/LOGIC_CORE $@
