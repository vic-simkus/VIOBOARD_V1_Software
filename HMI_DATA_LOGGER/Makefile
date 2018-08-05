
#
# This file is mechanically generated.  Any changes will most likely be lost.
#
# File generated on: 2018-07-29T11:51:00.599507
#

#
# Top-level project targets
#

all: HMI_DATA_LOGGER_all

clean: HMI_DATA_LOGGER_clean

HMI_DATA_LOGGER_all: HMI_DATA_LOGGER_all_o
	g++ -fexceptions  -o bin/HMI_DATA_LOGGER bin/hmi_data_logger.o bin/hmi_data_logger_context.o bin/hmi_data_logger_connection.o -L../HVAC_LIB/bin  -lHVAC_LIB  -pthread 

HMI_DATA_LOGGER_all_o: HMI_DATA_LOGGER_make_dirs bin/hmi_data_logger.o  bin/hmi_data_logger_context.o  bin/hmi_data_logger_connection.o  

HMI_DATA_LOGGER_clean: 
	@echo Cleaning stuff.
	- @rm bin/hmi_data_logger.o 2> /dev/null || true
	- @rm bin/hmi_data_logger_context.o 2> /dev/null || true
	- @rm bin/hmi_data_logger_connection.o 2> /dev/null || true
	- @rm bin/HMI_DATA_LOGGER 2> /dev/null || true

HMI_DATA_LOGGER_make_dirs: 
	- @mkdir -p bin/

bin/hmi_data_logger.o: src/hmi_data_logger.cpp \
 src/include/hmi_data_logger_context.hpp \
 src/include/hmi_data_logger_config.hpp \
 ../HVAC_LIB/src/include/lib/logger.hpp \
 ../HVAC_LIB/src/include/lib/hvac_types.hpp \
 ../HVAC_LIB/src/include/lib/tprotect_base.hpp \
 ../HVAC_LIB/src/include/lib/exceptions.hpp \
 ../HVAC_LIB/src/include/lib/string_lib.hpp \
 src/include/hmi_data_logger_connection.hpp \
 ../HVAC_LIB/src/include/lib/bbb_hvac.hpp \
 ../HVAC_LIB/src/include/lib/message_lib.hpp \
 ../HVAC_LIB/src/include/lib/message_types.hpp \
 ../HVAC_LIB/src/include/lib/message_processor.hpp \
 ../HVAC_LIB/src/include/lib/configurator.hpp \
 ../HVAC_LIB/src/include/lib/globals.hpp \
 ../HVAC_LIB/src/include/lib/config.hpp \
 ../HVAC_LIB/src/include/lib/context.hpp \
 ../HVAC_LIB/src/include/lib/message_callbacks.hpp \
 ../HVAC_LIB/src/include/lib/threads/thread_base.hpp \
 ../HVAC_LIB/src/include/lib/serial_io_types.hpp \
 ../HVAC_LIB/src/include/lib/outgoing_message_queue.hpp \
 ../HVAC_LIB/src/include/lib/logger.hpp \
 ../HVAC_LIB/src/include/lib/board_state_cache.hpp \
 ../HVAC_LIB/src/include/lib/serial_io_types.hpp \
 ../HVAC_LIB/src/include/lib/serial_io_type_operators.hpp
	g++ -I../HVAC_LIB/src/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/hmi_data_logger.cpp -o bin/hmi_data_logger.o

bin/hmi_data_logger_context.o: src/hmi_data_logger_context.cpp \
 src/include/hmi_data_logger_context.hpp \
 src/include/hmi_data_logger_config.hpp \
 ../HVAC_LIB/src/include/lib/logger.hpp \
 ../HVAC_LIB/src/include/lib/hvac_types.hpp \
 ../HVAC_LIB/src/include/lib/tprotect_base.hpp \
 ../HVAC_LIB/src/include/lib/exceptions.hpp \
 ../HVAC_LIB/src/include/lib/string_lib.hpp
	g++ -I../HVAC_LIB/src/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/hmi_data_logger_context.cpp -o bin/hmi_data_logger_context.o

bin/hmi_data_logger_connection.o: src/hmi_data_logger_connection.cpp \
 src/include/hmi_data_logger_connection.hpp \
 ../HVAC_LIB/src/include/lib/bbb_hvac.hpp \
 ../HVAC_LIB/src/include/lib/message_lib.hpp \
 ../HVAC_LIB/src/include/lib/message_types.hpp \
 ../HVAC_LIB/src/include/lib/hvac_types.hpp \
 ../HVAC_LIB/src/include/lib/exceptions.hpp \
 ../HVAC_LIB/src/include/lib/string_lib.hpp \
 ../HVAC_LIB/src/include/lib/message_processor.hpp \
 ../HVAC_LIB/src/include/lib/configurator.hpp \
 ../HVAC_LIB/src/include/lib/logger.hpp \
 ../HVAC_LIB/src/include/lib/tprotect_base.hpp \
 ../HVAC_LIB/src/include/lib/globals.hpp \
 ../HVAC_LIB/src/include/lib/config.hpp \
 ../HVAC_LIB/src/include/lib/context.hpp \
 ../HVAC_LIB/src/include/lib/message_callbacks.hpp \
 ../HVAC_LIB/src/include/lib/threads/thread_base.hpp \
 ../HVAC_LIB/src/include/lib/serial_io_types.hpp \
 ../HVAC_LIB/src/include/lib/outgoing_message_queue.hpp \
 ../HVAC_LIB/src/include/lib/logger.hpp \
 ../HVAC_LIB/src/include/lib/board_state_cache.hpp \
 ../HVAC_LIB/src/include/lib/serial_io_types.hpp \
 ../HVAC_LIB/src/include/lib/serial_io_type_operators.hpp
	g++ -I../HVAC_LIB/src/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/hmi_data_logger_connection.cpp -o bin/hmi_data_logger_connection.o

#
# EOF
#
		
