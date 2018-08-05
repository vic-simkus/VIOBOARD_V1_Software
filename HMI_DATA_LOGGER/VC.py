#!/usr/bin/env python

from make_makefile import SourceFile
from make_makefile import Context

import os

class MyContext(Context):
	SOURCE_FILES = (
			SourceFile("hmi_data_logger.cpp"),
			SourceFile("hmi_data_logger_context.cpp"),
			SourceFile("hmi_data_logger_connection.cpp"),
			)

	TAG = "HMI_DATA_LOGGER"

	EXE_TARGET=os.path.join(Context.OUTPUT_DIR,"HMI_DATA_LOGGER")

	RELATED_PROJECTS=("../HVAC_LIB",)


def vc_init():
	return MyContext()
