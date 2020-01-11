#!/usr/bin/env python

from make_makefile import SourceFile
from make_makefile import Context

import os

class MyContext(Context):
	SOURCE_FILES = (
			SourceFile("bbb_hvac.cpp"),SourceFile("command_line_parms.cpp"),
			)
	TAG = "LOGIC_CORE"

#	INCLUDE_DIRS=(os.path.join(Context.SOURCE_DIR,"include"),None)

	EXE_TARGET=os.path.join(Context.OUTPUT_DIR,"LOGIC_CORE")

	RELATED_PROJECTS=("../HVAC_LIB",)


def vc_init():
	return MyContext()
