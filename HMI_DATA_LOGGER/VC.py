#!/usr/bin/env python

from make_makefile import SourceFile
from make_makefile import Context

import os

class MyContext(Context):
	SOURCE_FILES = (
			SourceFile("HmiDataLogger.cpp"),
			SourceFile("Context.cpp"),
			SourceFile("Config.cpp"),
			SourceFile("Connection.cpp"),
			SourceFile("ConnectionFile.cpp"),
			SourceFile("ConnectionPgsql.cpp"),
			)

	TAG = "HMI_DATA_LOGGER"

	EXE_TARGET=os.path.join(Context.OUTPUT_DIR,"HMI_DATA_LOGGER")

	RELATED_PROJECTS=("../HVAC_LIB",)


def vc_init():
	return MyContext()
