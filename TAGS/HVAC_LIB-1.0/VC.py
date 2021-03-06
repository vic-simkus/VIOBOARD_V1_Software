#!/usr/bin/env python


# This file is part of the software stack for Vic's IO board and its
# associated projects.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.

# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Copyright 2016,2017,2018 Vidas Simkus (vic.simkus@gmail.com)



from make_makefile import SourceFile
from make_makefile import Context
from make_makefile import CLANGContext
from make_makefile import GCCContext

import os

class MyContext(CLANGContext):
	def __init__(self):
		super(MyContext,self).__init__()
		self.CXX_FLAGS.append("-Wno-deprecated");
		self.LIB_TARGET="HVAC_LIB"
		self.TAG = self.LIB_TARGET

	SOURCE_FILES = (
			SourceFile("context/base_context.cpp"),
			SourceFile("context/client_context.cpp"),
			SourceFile("context/context.cpp"),
			SourceFile("threads/HVAC_logic_loop.cpp"),
			SourceFile("threads/logic_thread.cpp"),
			SourceFile("threads/serial_io_thread.cpp"),
			SourceFile("threads/shim_listener_thread.cpp"),
			SourceFile("threads/thread_base.cpp"),
			SourceFile("threads/thread_registry.cpp"),
			SourceFile("threads/tprotect_base.cpp"),
			SourceFile("threads/watchdog_thread.cpp"),
			SourceFile("config_entry.cpp"),
			SourceFile("configurator.cpp"),
			SourceFile("exceptions.cpp"),
			SourceFile("globals.cpp"),
			SourceFile("logger.cpp"),
			SourceFile("log_configurator.cpp"),
			SourceFile("board_state_cache.cpp"),
			SourceFile("message_callbacks.cpp"),
			SourceFile("message_lib.cpp"),
			SourceFile("message_processor.cpp"),
			SourceFile("message_types.cpp"),
			SourceFile("serial_io_types.cpp"),
			SourceFile("socket_reader.cpp"),
			SourceFile("string_lib.cpp"),
			SourceFile("command_line_parms.cpp"),
			SourceFile("configurator/set_point.cpp"),
			SourceFile("configurator/board_point.cpp"),
			SourceFile("configurator/configurator_operators.cpp")
			)

	INCLUDE_DIRS=(os.path.join(Context.SOURCE_DIR,"include"),)


def vc_init():
	return MyContext()
