
#
# This file is mechanically generated.  Any changes will most likely be lost.
#
# File generated on: 2020-05-09T21:35:43.401717
#

#
# Top-level project targets
#

all: LOGIC_CORE_all

clean: LOGIC_CORE_clean

LOGIC_CORE_all: LOGIC_CORE_all_o
	clang-8   -o bin/LOGIC_CORE  bin/bbb_hvac.o -L../HVAC_LIB/bin  -lHVAC_LIB  -lpthread  -lstdc++

LOGIC_CORE_all_o: LOGIC_CORE_make_dirs bin/bbb_hvac.o  

LOGIC_CORE_clean: 
	@echo Cleaning stuff.
	- @rm bin/bbb_hvac.o 2> /dev/null || true
	- @rm bin/LOGIC_CORE 2> /dev/null || true

LOGIC_CORE_make_dirs: 
	- @mkdir -p bin/

bin/bbb_hvac.o: src/bbb_hvac.cpp ../HVAC_LIB/src/include/lib/bbb_hvac.hpp \
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
  ../HVAC_LIB/src/include/lib/socket_reader.hpp \
  ../HVAC_LIB/src/include/lib/threads/logic_thread.hpp \
  ../HVAC_LIB/src/include/lib/threads/HVAC_logic_loop.hpp \
  ../HVAC_LIB/src/include/lib/threads/serial_io_thread.hpp \
  ../HVAC_LIB/src/include/lib/threads/shim_listener_thread.hpp \
  ../HVAC_LIB/src/include/lib/threads/thread_registry.hpp \
  ../HVAC_LIB/src/include/lib/log_configurator.hpp \
  ../HVAC_LIB/src/include/lib/command_line_parms.h
	clang-8 -I../HVAC_LIB/src/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++17 -fPIC -fexceptions -Wno-deprecated -pthread  -c src/bbb_hvac.cpp -o bin/bbb_hvac.o

#
# EOF
#
		
