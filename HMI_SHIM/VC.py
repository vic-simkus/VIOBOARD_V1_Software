#!/usr/bin/env python

from make_makefile import SourceFile
from make_makefile import Context

import os

class MyContext(Context):
	SOURCE_FILES = (
			SourceFile("hmi_shim.cpp"),
			)

	TAG = "HMI_SHIM"

	EXE_TARGET=os.path.join(Context.OUTPUT_DIR,"HMI_SHIM")

	RELATED_PROJECTS=("../HVAC_LIB",)


def vc_init():
	return MyContext()
