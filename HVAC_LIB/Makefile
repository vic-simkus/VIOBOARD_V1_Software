
#
# This file is mechanically generated.  Any changes will most likely be lost.
#
# File generated on: 2017-11-11T23:07:48.745621
#

#
# Top-level project targets
#

all: HVAC_LIB_all

clean: HVAC_LIB_clean

HVAC_LIB_all: HVAC_LIB_all_o
	g++ -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -shared -pthread  -o bin/libHVAC_LIB.so  bin/context/base_context.o bin/context/client_context.o bin/context/context.o bin/threads/HVAC_logic_loop.o bin/threads/logic_thread.o bin/threads/serial_io_thread.o bin/threads/shim_listener_thread.o bin/threads/thread_base.o bin/threads/thread_registry.o bin/threads/tprotect_base.o bin/threads/watchdog_thread.o bin/config_entry.o bin/configurator.o bin/exceptions.o bin/globals.o bin/logger.o bin/message_callbacks.o bin/message_lib.o bin/message_processor.o bin/message_types.o bin/serial_io_types.o bin/socket_reader.o bin/string_lib.o

HVAC_LIB_all_o: HVAC_LIB_make_dirs bin/context/base_context.o  bin/context/client_context.o  bin/context/context.o  bin/threads/HVAC_logic_loop.o  bin/threads/logic_thread.o  bin/threads/serial_io_thread.o  bin/threads/shim_listener_thread.o  bin/threads/thread_base.o  bin/threads/thread_registry.o  bin/threads/tprotect_base.o  bin/threads/watchdog_thread.o  bin/config_entry.o  bin/configurator.o  bin/exceptions.o  bin/globals.o  bin/logger.o  bin/message_callbacks.o  bin/message_lib.o  bin/message_processor.o  bin/message_types.o  bin/serial_io_types.o  bin/socket_reader.o  bin/string_lib.o  

HVAC_LIB_clean: 
	@echo Cleaning stuff.
	- @rm bin/context/base_context.o 2> /dev/null || true
	- @rm bin/context/client_context.o 2> /dev/null || true
	- @rm bin/context/context.o 2> /dev/null || true
	- @rm bin/threads/HVAC_logic_loop.o 2> /dev/null || true
	- @rm bin/threads/logic_thread.o 2> /dev/null || true
	- @rm bin/threads/serial_io_thread.o 2> /dev/null || true
	- @rm bin/threads/shim_listener_thread.o 2> /dev/null || true
	- @rm bin/threads/thread_base.o 2> /dev/null || true
	- @rm bin/threads/thread_registry.o 2> /dev/null || true
	- @rm bin/threads/tprotect_base.o 2> /dev/null || true
	- @rm bin/threads/watchdog_thread.o 2> /dev/null || true
	- @rm bin/config_entry.o 2> /dev/null || true
	- @rm bin/configurator.o 2> /dev/null || true
	- @rm bin/exceptions.o 2> /dev/null || true
	- @rm bin/globals.o 2> /dev/null || true
	- @rm bin/logger.o 2> /dev/null || true
	- @rm bin/message_callbacks.o 2> /dev/null || true
	- @rm bin/message_lib.o 2> /dev/null || true
	- @rm bin/message_processor.o 2> /dev/null || true
	- @rm bin/message_types.o 2> /dev/null || true
	- @rm bin/serial_io_types.o 2> /dev/null || true
	- @rm bin/socket_reader.o 2> /dev/null || true
	- @rm bin/string_lib.o 2> /dev/null || true
	- @rm HVAC_LIB 2> /dev/null || true

HVAC_LIB_make_dirs: 
	- @mkdir -p bin/context
	- @mkdir -p bin/threads
	- @mkdir -p bin/

bin/context/base_context.o: src/context/base_context.cpp \
 src/include/lib/context.hpp src/include/lib/logger.hpp \
 src/include/lib/hvac_types.hpp src/include/lib/tprotect_base.hpp \
 src/include/lib/exceptions.hpp src/include/lib/string_lib.hpp \
 src/include/lib/message_callbacks.hpp src/include/lib/message_lib.hpp \
 src/include/lib/message_types.hpp \
 src/include/lib/threads/thread_base.hpp src/include/lib/config.hpp \
 src/include/lib/message_processor.hpp src/include/lib/configurator.hpp \
 src/include/lib/socket_reader.hpp src/include/lib/exceptions.hpp \
 src/include/lib/globals.hpp src/include/lib/threads/logic_thread.hpp \
 src/include/lib/threads/serial_io_thread.hpp \
 src/include/lib/serial_io_types.hpp \
 src/include/lib/outgoing_message_queue.hpp src/include/lib/logger.hpp \
 src/include/lib/board_state_cache.hpp \
 src/include/lib/serial_io_types.hpp \
 src/include/lib/serial_io_type_operators.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/context/base_context.cpp -o bin/context/base_context.o

bin/context/client_context.o: src/context/client_context.cpp \
 src/include/lib/context.hpp src/include/lib/logger.hpp \
 src/include/lib/hvac_types.hpp src/include/lib/tprotect_base.hpp \
 src/include/lib/exceptions.hpp src/include/lib/string_lib.hpp \
 src/include/lib/message_callbacks.hpp src/include/lib/message_lib.hpp \
 src/include/lib/message_types.hpp \
 src/include/lib/threads/thread_base.hpp src/include/lib/config.hpp \
 src/include/lib/message_processor.hpp src/include/lib/configurator.hpp \
 src/include/lib/socket_reader.hpp src/include/lib/exceptions.hpp \
 src/include/lib/globals.hpp src/include/lib/threads/logic_thread.hpp \
 src/include/lib/threads/serial_io_thread.hpp \
 src/include/lib/serial_io_types.hpp \
 src/include/lib/outgoing_message_queue.hpp src/include/lib/logger.hpp \
 src/include/lib/board_state_cache.hpp \
 src/include/lib/serial_io_types.hpp \
 src/include/lib/serial_io_type_operators.hpp \
 src/include/lib/threads/watchdog_thread.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/context/client_context.cpp -o bin/context/client_context.o

bin/context/context.o: src/context/context.cpp \
 src/include/lib/context.hpp src/include/lib/logger.hpp \
 src/include/lib/hvac_types.hpp src/include/lib/tprotect_base.hpp \
 src/include/lib/exceptions.hpp src/include/lib/string_lib.hpp \
 src/include/lib/message_callbacks.hpp src/include/lib/message_lib.hpp \
 src/include/lib/message_types.hpp \
 src/include/lib/threads/thread_base.hpp src/include/lib/config.hpp \
 src/include/lib/message_processor.hpp src/include/lib/configurator.hpp \
 src/include/lib/socket_reader.hpp src/include/lib/exceptions.hpp \
 src/include/lib/globals.hpp src/include/lib/threads/logic_thread.hpp \
 src/include/lib/threads/serial_io_thread.hpp \
 src/include/lib/serial_io_types.hpp \
 src/include/lib/outgoing_message_queue.hpp src/include/lib/logger.hpp \
 src/include/lib/board_state_cache.hpp \
 src/include/lib/serial_io_types.hpp \
 src/include/lib/serial_io_type_operators.hpp \
 src/include/lib/threads/thread_registry.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/context/context.cpp -o bin/context/context.o

bin/threads/HVAC_logic_loop.o: src/threads/HVAC_logic_loop.cpp \
 src/include/lib/threads/HVAC_logic_loop.hpp \
 src/include/lib/threads/logic_thread.hpp src/include/lib/logger.hpp \
 src/include/lib/hvac_types.hpp src/include/lib/tprotect_base.hpp \
 src/include/lib/exceptions.hpp src/include/lib/string_lib.hpp \
 src/include/lib/config.hpp src/include/lib/threads/thread_base.hpp \
 src/include/lib/configurator.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/threads/HVAC_logic_loop.cpp -o bin/threads/HVAC_logic_loop.o

bin/threads/logic_thread.o: src/threads/logic_thread.cpp \
 src/include/lib/exceptions.hpp src/include/lib/string_lib.hpp \
 src/include/lib/config.hpp src/include/lib/globals.hpp \
 src/include/lib/configurator.hpp src/include/lib/logger.hpp \
 src/include/lib/hvac_types.hpp src/include/lib/tprotect_base.hpp \
 src/include/lib/threads/logic_thread.hpp \
 src/include/lib/threads/thread_base.hpp \
 src/include/lib/threads/watchdog_thread.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/threads/logic_thread.cpp -o bin/threads/logic_thread.o

bin/threads/serial_io_thread.o: src/threads/serial_io_thread.cpp \
 src/include/lib/threads/serial_io_thread.hpp src/include/lib/config.hpp \
 src/include/lib/logger.hpp src/include/lib/hvac_types.hpp \
 src/include/lib/tprotect_base.hpp src/include/lib/exceptions.hpp \
 src/include/lib/string_lib.hpp src/include/lib/serial_io_types.hpp \
 src/include/lib/outgoing_message_queue.hpp src/include/lib/logger.hpp \
 src/include/lib/board_state_cache.hpp \
 src/include/lib/serial_io_types.hpp \
 src/include/lib/serial_io_type_operators.hpp \
 src/include/lib/threads/thread_base.hpp src/include/lib/globals.hpp \
 src/include/lib/memory_management.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/threads/serial_io_thread.cpp -o bin/threads/serial_io_thread.o

bin/threads/shim_listener_thread.o: src/threads/shim_listener_thread.cpp \
 src/include/lib/threads/shim_listener_thread.hpp \
 src/include/lib/logger.hpp src/include/lib/hvac_types.hpp \
 src/include/lib/tprotect_base.hpp src/include/lib/exceptions.hpp \
 src/include/lib/string_lib.hpp src/include/lib/threads/thread_base.hpp \
 src/include/lib/context.hpp src/include/lib/message_callbacks.hpp \
 src/include/lib/message_lib.hpp src/include/lib/message_types.hpp \
 src/include/lib/globals.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/threads/shim_listener_thread.cpp -o bin/threads/shim_listener_thread.o

bin/threads/thread_base.o: src/threads/thread_base.cpp \
 src/include/lib/threads/thread_base.hpp src/include/lib/exceptions.hpp \
 src/include/lib/string_lib.hpp src/include/lib/tprotect_base.hpp \
 src/include/lib/threads/thread_registry.hpp src/include/lib/logger.hpp \
 src/include/lib/hvac_types.hpp \
 src/include/lib/threads/serial_io_thread.hpp src/include/lib/config.hpp \
 src/include/lib/serial_io_types.hpp \
 src/include/lib/outgoing_message_queue.hpp src/include/lib/logger.hpp \
 src/include/lib/board_state_cache.hpp \
 src/include/lib/serial_io_types.hpp \
 src/include/lib/serial_io_type_operators.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/threads/thread_base.cpp -o bin/threads/thread_base.o

bin/threads/thread_registry.o: src/threads/thread_registry.cpp \
 src/include/lib/threads/thread_registry.hpp \
 src/include/lib/tprotect_base.hpp src/include/lib/exceptions.hpp \
 src/include/lib/string_lib.hpp src/include/lib/logger.hpp \
 src/include/lib/hvac_types.hpp \
 src/include/lib/threads/serial_io_thread.hpp src/include/lib/config.hpp \
 src/include/lib/serial_io_types.hpp \
 src/include/lib/outgoing_message_queue.hpp src/include/lib/logger.hpp \
 src/include/lib/board_state_cache.hpp \
 src/include/lib/serial_io_types.hpp \
 src/include/lib/serial_io_type_operators.hpp \
 src/include/lib/threads/thread_base.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/threads/thread_registry.cpp -o bin/threads/thread_registry.o

bin/threads/tprotect_base.o: src/threads/tprotect_base.cpp \
 src/include/lib/tprotect_base.hpp src/include/lib/exceptions.hpp \
 src/include/lib/string_lib.hpp src/include/lib/logger.hpp \
 src/include/lib/hvac_types.hpp src/include/lib/config.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/threads/tprotect_base.cpp -o bin/threads/tprotect_base.o

bin/threads/watchdog_thread.o: src/threads/watchdog_thread.cpp \
 src/include/lib/config.hpp src/include/lib/logger.hpp \
 src/include/lib/hvac_types.hpp src/include/lib/tprotect_base.hpp \
 src/include/lib/exceptions.hpp src/include/lib/string_lib.hpp \
 src/include/lib/threads/watchdog_thread.hpp \
 src/include/lib/threads/thread_base.hpp \
 src/include/lib/threads/thread_registry.hpp \
 src/include/lib/threads/serial_io_thread.hpp \
 src/include/lib/serial_io_types.hpp \
 src/include/lib/outgoing_message_queue.hpp src/include/lib/logger.hpp \
 src/include/lib/board_state_cache.hpp \
 src/include/lib/serial_io_types.hpp \
 src/include/lib/serial_io_type_operators.hpp src/include/lib/globals.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/threads/watchdog_thread.cpp -o bin/threads/watchdog_thread.o

bin/config_entry.o: src/config_entry.cpp src/include/lib/configurator.hpp \
 src/include/lib/logger.hpp src/include/lib/hvac_types.hpp \
 src/include/lib/tprotect_base.hpp src/include/lib/exceptions.hpp \
 src/include/lib/string_lib.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/config_entry.cpp -o bin/config_entry.o

bin/configurator.o: src/configurator.cpp src/include/lib/configurator.hpp \
 src/include/lib/logger.hpp src/include/lib/hvac_types.hpp \
 src/include/lib/tprotect_base.hpp src/include/lib/exceptions.hpp \
 src/include/lib/string_lib.hpp src/include/lib/config.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/configurator.cpp -o bin/configurator.o

bin/exceptions.o: src/exceptions.cpp src/include/lib/exceptions.hpp \
 src/include/lib/string_lib.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/exceptions.cpp -o bin/exceptions.o

bin/globals.o: src/globals.cpp src/include/lib/globals.hpp \
 src/include/lib/config.hpp src/include/lib/exceptions.hpp \
 src/include/lib/string_lib.hpp \
 src/include/lib/threads/watchdog_thread.hpp \
 src/include/lib/threads/thread_base.hpp \
 src/include/lib/tprotect_base.hpp src/include/lib/logger.hpp \
 src/include/lib/hvac_types.hpp \
 src/include/lib/threads/thread_registry.hpp \
 src/include/lib/threads/serial_io_thread.hpp \
 src/include/lib/serial_io_types.hpp \
 src/include/lib/outgoing_message_queue.hpp src/include/lib/logger.hpp \
 src/include/lib/board_state_cache.hpp \
 src/include/lib/serial_io_types.hpp \
 src/include/lib/serial_io_type_operators.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/globals.cpp -o bin/globals.o

bin/logger.o: src/logger.cpp src/include/lib/logger.hpp \
 src/include/lib/hvac_types.hpp src/include/lib/tprotect_base.hpp \
 src/include/lib/exceptions.hpp src/include/lib/string_lib.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/logger.cpp -o bin/logger.o

bin/message_callbacks.o: src/message_callbacks.cpp \
 src/include/lib/message_callbacks.hpp src/include/lib/logger.hpp \
 src/include/lib/hvac_types.hpp src/include/lib/tprotect_base.hpp \
 src/include/lib/exceptions.hpp src/include/lib/string_lib.hpp \
 src/include/lib/message_lib.hpp src/include/lib/message_types.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/message_callbacks.cpp -o bin/message_callbacks.o

bin/message_lib.o: src/message_lib.cpp src/include/lib/message_lib.hpp \
 src/include/lib/message_types.hpp src/include/lib/hvac_types.hpp \
 src/include/lib/exceptions.hpp src/include/lib/string_lib.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/message_lib.cpp -o bin/message_lib.o

bin/message_processor.o: src/message_processor.cpp \
 src/include/lib/message_processor.hpp src/include/lib/message_lib.hpp \
 src/include/lib/message_types.hpp src/include/lib/hvac_types.hpp \
 src/include/lib/exceptions.hpp src/include/lib/string_lib.hpp \
 src/include/lib/configurator.hpp src/include/lib/logger.hpp \
 src/include/lib/tprotect_base.hpp src/include/lib/config.hpp \
 src/include/lib/threads/logic_thread.hpp \
 src/include/lib/threads/thread_base.hpp src/include/lib/globals.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/message_processor.cpp -o bin/message_processor.o

bin/message_types.o: src/message_types.cpp \
 src/include/lib/message_types.hpp src/include/lib/hvac_types.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/message_types.cpp -o bin/message_types.o

bin/serial_io_types.o: src/serial_io_types.cpp \
 src/include/lib/serial_io_types.hpp src/include/lib/config.hpp \
 src/include/lib/tprotect_base.hpp src/include/lib/exceptions.hpp \
 src/include/lib/string_lib.hpp \
 src/include/lib/outgoing_message_queue.hpp src/include/lib/logger.hpp \
 src/include/lib/hvac_types.hpp src/include/lib/board_state_cache.hpp \
 src/include/lib/serial_io_types.hpp \
 src/include/lib/serial_io_type_operators.hpp src/include/lib/logger.hpp \
 src/include/lib/memory_management.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/serial_io_types.cpp -o bin/serial_io_types.o

bin/socket_reader.o: src/socket_reader.cpp \
 src/include/lib/socket_reader.hpp src/include/lib/exceptions.hpp \
 src/include/lib/string_lib.hpp src/include/lib/config.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/socket_reader.cpp -o bin/socket_reader.o

bin/string_lib.o: src/string_lib.cpp src/include/lib/string_lib.hpp
	g++ -Isrc/include  -g -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wunused-parameter -Wsign-compare -std=c++11 -fPIC -fexceptions -pthread  -c src/string_lib.cpp -o bin/string_lib.o

#
# EOF
#
		
