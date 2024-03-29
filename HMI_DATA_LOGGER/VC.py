#!/usr/bin/env python

from make_makefile import SourceFile
from make_makefile import Context
from make_makefile import GCCContext
from make_makefile import CLANGContext

import os
import sys

class MyContext(CLANGContext):
	def __init__(self):
		super(MyContext,self).__init__()
		if sys.platform == "freebsd11":
			self.INCLUDE_DIRS += ["/usr/local/include",]
			self.LIB_DIRS += ["/usr/local/lib",]
		else:
			self.INCLUDE_DIRS += ["/usr/include/postgresql",]
		
	SOURCE_FILES = (
			SourceFile("HmiDataLogger.cpp"),
			SourceFile("Context.cpp"),
			SourceFile("Config.cpp"),
			SourceFile("Connection.cpp"),
			SourceFile("ConnectionFile.cpp"),
			SourceFile("ConnectionPgsql.cpp"),
			SourceFile("Exception.cpp")
			)

	TAG = "HMI_DATA_LOGGER"

	EXE_TARGET=os.path.join(Context.OUTPUT_DIR,"HMI_DATA_LOGGER")

	RELATED_PROJECTS=("../HVAC_LIB",)

	LIBRARIES = ["pq","systemd"]


        
def vc_init():
	return MyContext()


if __name__ == "main":
	vc_init()
