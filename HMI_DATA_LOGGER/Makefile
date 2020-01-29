
#
# This file is mechanically generated.  Any changes will most likely be lost.
#
# File generated on: 2020-01-25T23:23:45.927043
#

#
# Top-level project targets
#

all: HMI_DATA_LOGGER_all

clean: HMI_DATA_LOGGER_clean

HMI_DATA_LOGGER_all: HMI_DATA_LOGGER_all_o
	clang-8   -o bin/HMI_DATA_LOGGER  bin/HmiDataLogger.o bin/Context.o bin/Config.o bin/Connection.o bin/ConnectionFile.o bin/ConnectionPgsql.o -L../HVAC_LIB/bin  -lpq -lHVAC_LIB  -lpthread  -lstdc++

HMI_DATA_LOGGER_all_o: HMI_DATA_LOGGER_make_dirs bin/HmiDataLogger.o  bin/Context.o  bin/Config.o  bin/Connection.o  bin/ConnectionFile.o  bin/ConnectionPgsql.o  

HMI_DATA_LOGGER_clean: 
	@echo Cleaning stuff.
	- @rm bin/HmiDataLogger.o 2> /dev/null || true
	- @rm bin/Context.o 2> /dev/null || true
	- @rm bin/Config.o 2> /dev/null || true
	- @rm bin/Connection.o 2> /dev/null || true
	- @rm bin/ConnectionFile.o 2> /dev/null || true
	- @rm bin/ConnectionPgsql.o 2> /dev/null || true
	- @rm bin/HMI_DATA_LOGGER 2> /dev/null || true

HMI_DATA_LOGGER_make_dirs: 
	- @mkdir -p bin/

bin/HmiDataLogger.o: src/HmiDataLogger.cpp src/include/Context.hpp \
  src/include/Config.hpp \
  ../HVAC_LIB/src/include/lib/command_line_parms.h \
  ../HVAC_LIB/src/include/lib/context.hpp \
  ../HVAC_LIB/src/include/lib/logger.hpp \
  ../HVAC_LIB/src/include/lib/hvac_types.hpp \
  ../HVAC_LIB/src/include/lib/exceptions.hpp \
  ../HVAC_LIB/src/include/lib/string_lib.hpp \
  ../HVAC_LIB/src/include/lib/message_callbacks.hpp \
  ../HVAC_LIB/src/include/lib/message_lib.hpp \
  ../HVAC_LIB/src/include/lib/message_types.hpp \
  ../HVAC_LIB/src/include/lib/tprotect_base.hpp \
  ../HVAC_LIB/src/include/lib/threads/thread_base.hpp \
  ../HVAC_LIB/src/include/lib/config.hpp src/include/ConnectionFile.hpp \
  src/include/Connection.hpp ../HVAC_LIB/src/include/lib/bbb_hvac.hpp \
  ../HVAC_LIB/src/include/lib/message_processor.hpp \
  ../HVAC_LIB/src/include/lib/configurator.hpp \
  ../HVAC_LIB/src/include/lib/configurator/configurator_types.hpp \
  ../HVAC_LIB/src/include/lib/globals.hpp \
  ../HVAC_LIB/src/include/lib/serial_io_types.hpp \
  ../HVAC_LIB/src/include/lib/outgoing_message_queue.hpp \
  ../HVAC_LIB/src/include/lib/board_state_cache.hpp \
  ../HVAC_LIB/src/include/lib/serial_io_type_operators.hpp \
  src/include/ConnectionPgsql.hpp \
  ../HVAC_LIB/src/include/lib/threads/thread_registry.hpp \
  ../HVAC_LIB/src/include/lib/threads/serial_io_thread.hpp
	clang-8 -I../HVAC_LIB/src/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -Wno-deprecated -pthread  -c src/HmiDataLogger.cpp -o bin/HmiDataLogger.o

bin/Context.o: src/Context.cpp src/include/Context.hpp \
  src/include/Config.hpp \
  ../HVAC_LIB/src/include/lib/command_line_parms.h \
  ../HVAC_LIB/src/include/lib/context.hpp \
  ../HVAC_LIB/src/include/lib/logger.hpp \
  ../HVAC_LIB/src/include/lib/hvac_types.hpp \
  ../HVAC_LIB/src/include/lib/exceptions.hpp \
  ../HVAC_LIB/src/include/lib/string_lib.hpp \
  ../HVAC_LIB/src/include/lib/message_callbacks.hpp \
  ../HVAC_LIB/src/include/lib/message_lib.hpp \
  ../HVAC_LIB/src/include/lib/message_types.hpp \
  ../HVAC_LIB/src/include/lib/tprotect_base.hpp \
  ../HVAC_LIB/src/include/lib/threads/thread_base.hpp \
  ../HVAC_LIB/src/include/lib/config.hpp
	clang-8 -I../HVAC_LIB/src/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -Wno-deprecated -pthread  -c src/Context.cpp -o bin/Context.o

bin/Config.o: src/Config.cpp src/include/Config.hpp \
  ../HVAC_LIB/src/include/lib/command_line_parms.h \
  ../HVAC_LIB/src/include/lib/context.hpp \
  ../HVAC_LIB/src/include/lib/logger.hpp \
  ../HVAC_LIB/src/include/lib/hvac_types.hpp \
  ../HVAC_LIB/src/include/lib/exceptions.hpp \
  ../HVAC_LIB/src/include/lib/string_lib.hpp \
  ../HVAC_LIB/src/include/lib/message_callbacks.hpp \
  ../HVAC_LIB/src/include/lib/message_lib.hpp \
  ../HVAC_LIB/src/include/lib/message_types.hpp \
  ../HVAC_LIB/src/include/lib/tprotect_base.hpp \
  ../HVAC_LIB/src/include/lib/threads/thread_base.hpp \
  ../HVAC_LIB/src/include/lib/config.hpp src/include/Context.hpp
	clang-8 -I../HVAC_LIB/src/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -Wno-deprecated -pthread  -c src/Config.cpp -o bin/Config.o

bin/Connection.o: src/Connection.cpp src/include/Connection.hpp \
  ../HVAC_LIB/src/include/lib/bbb_hvac.hpp \
  ../HVAC_LIB/src/include/lib/message_lib.hpp \
  ../HVAC_LIB/src/include/lib/message_types.hpp \
  ../HVAC_LIB/src/include/lib/hvac_types.hpp \
  ../HVAC_LIB/src/include/lib/exceptions.hpp \
  ../HVAC_LIB/src/include/lib/string_lib.hpp \
  ../HVAC_LIB/src/include/lib/message_processor.hpp \
  ../HVAC_LIB/src/include/lib/configurator.hpp \
  ../HVAC_LIB/src/include/lib/logger.hpp \
  ../HVAC_LIB/src/include/lib/configurator/configurator_types.hpp \
  ../HVAC_LIB/src/include/lib/globals.hpp \
  ../HVAC_LIB/src/include/lib/config.hpp \
  ../HVAC_LIB/src/include/lib/context.hpp \
  ../HVAC_LIB/src/include/lib/message_callbacks.hpp \
  ../HVAC_LIB/src/include/lib/tprotect_base.hpp \
  ../HVAC_LIB/src/include/lib/threads/thread_base.hpp \
  ../HVAC_LIB/src/include/lib/serial_io_types.hpp \
  ../HVAC_LIB/src/include/lib/outgoing_message_queue.hpp \
  ../HVAC_LIB/src/include/lib/board_state_cache.hpp \
  ../HVAC_LIB/src/include/lib/serial_io_type_operators.hpp \
  src/include/Context.hpp src/include/Config.hpp \
  ../HVAC_LIB/src/include/lib/command_line_parms.h
	clang-8 -I../HVAC_LIB/src/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -Wno-deprecated -pthread  -c src/Connection.cpp -o bin/Connection.o

bin/ConnectionFile.o: src/ConnectionFile.cpp \
  src/include/ConnectionFile.hpp src/include/Connection.hpp \
  ../HVAC_LIB/src/include/lib/bbb_hvac.hpp \
  ../HVAC_LIB/src/include/lib/message_lib.hpp \
  ../HVAC_LIB/src/include/lib/message_types.hpp \
  ../HVAC_LIB/src/include/lib/hvac_types.hpp \
  ../HVAC_LIB/src/include/lib/exceptions.hpp \
  ../HVAC_LIB/src/include/lib/string_lib.hpp \
  ../HVAC_LIB/src/include/lib/message_processor.hpp \
  ../HVAC_LIB/src/include/lib/configurator.hpp \
  ../HVAC_LIB/src/include/lib/logger.hpp \
  ../HVAC_LIB/src/include/lib/configurator/configurator_types.hpp \
  ../HVAC_LIB/src/include/lib/globals.hpp \
  ../HVAC_LIB/src/include/lib/config.hpp \
  ../HVAC_LIB/src/include/lib/context.hpp \
  ../HVAC_LIB/src/include/lib/message_callbacks.hpp \
  ../HVAC_LIB/src/include/lib/tprotect_base.hpp \
  ../HVAC_LIB/src/include/lib/threads/thread_base.hpp \
  ../HVAC_LIB/src/include/lib/serial_io_types.hpp \
  ../HVAC_LIB/src/include/lib/outgoing_message_queue.hpp \
  ../HVAC_LIB/src/include/lib/board_state_cache.hpp \
  ../HVAC_LIB/src/include/lib/serial_io_type_operators.hpp \
  src/include/Context.hpp src/include/Config.hpp \
  ../HVAC_LIB/src/include/lib/command_line_parms.h
	clang-8 -I../HVAC_LIB/src/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -Wno-deprecated -pthread  -c src/ConnectionFile.cpp -o bin/ConnectionFile.o

bin/ConnectionPgsql.o: src/ConnectionPgsql.cpp \
  src/include/ConnectionPgsql.hpp src/include/Connection.hpp \
  ../HVAC_LIB/src/include/lib/bbb_hvac.hpp \
  ../HVAC_LIB/src/include/lib/message_lib.hpp \
  ../HVAC_LIB/src/include/lib/message_types.hpp \
  ../HVAC_LIB/src/include/lib/hvac_types.hpp \
  ../HVAC_LIB/src/include/lib/exceptions.hpp \
  ../HVAC_LIB/src/include/lib/string_lib.hpp \
  ../HVAC_LIB/src/include/lib/message_processor.hpp \
  ../HVAC_LIB/src/include/lib/configurator.hpp \
  ../HVAC_LIB/src/include/lib/logger.hpp \
  ../HVAC_LIB/src/include/lib/configurator/configurator_types.hpp \
  ../HVAC_LIB/src/include/lib/globals.hpp \
  ../HVAC_LIB/src/include/lib/config.hpp \
  ../HVAC_LIB/src/include/lib/context.hpp \
  ../HVAC_LIB/src/include/lib/message_callbacks.hpp \
  ../HVAC_LIB/src/include/lib/tprotect_base.hpp \
  ../HVAC_LIB/src/include/lib/threads/thread_base.hpp \
  ../HVAC_LIB/src/include/lib/serial_io_types.hpp \
  ../HVAC_LIB/src/include/lib/outgoing_message_queue.hpp \
  ../HVAC_LIB/src/include/lib/board_state_cache.hpp \
  ../HVAC_LIB/src/include/lib/serial_io_type_operators.hpp
	clang-8 -I../HVAC_LIB/src/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -Wno-deprecated -pthread  -c src/ConnectionPgsql.cpp -o bin/ConnectionPgsql.o

#
# EOF
#
		
