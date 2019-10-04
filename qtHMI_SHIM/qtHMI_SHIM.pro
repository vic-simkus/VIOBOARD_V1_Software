release {
	DESTDIR = build/release
}

debug {
	DESTDIR = build/debug	
}

HEADERS = src/widgets/AI_RAW_VALUE.h \
	src/widgets/AI_VALUE.h \
	src/windows/BOARD_INFO_WIDGET.h \
	src/widgets/CAL_VALUE.h \
	src/widgets/DO_INDICATOR.h \
	src/windows/LOGIC_INFO.h \
	src/windows/MAIN_WINDOW.h \
	src/widgets/PMIC_INDICATOR.h \
	src/windows/RAW_BOARD_INFO.h \
	src/windows/DEBUG_FRAME.h \
	src/windows/DEBUG_WIDGET.h

SOURCES = src/widgets/AI_RAW_VALUE.cpp \
	src/windows/BOARD_INFO_WIDGET.cpp \
	src/widgets/CAL_VALUE.cpp \
	src/widgets/DO_INDICATOR.cpp \
	src/windows/LOGIC_INFO.cpp \
	src/main.cpp \
	src/windows/MAIN_WINDOW.cpp \
	src/widgets/PMIC_INDICATOR.cpp \
	src/windows/RAW_BOARD_INFO.cpp \
	src/widgets/AI_VALUE.cpp \
	src/windows/DEBUG_FRAME.cpp \
	src/windows/DEBUG_WIDGET.cpp

LIBS += -L../HVAC_LIB/bin -lHVAC_LIB
INCLUDEPATH += "../HVAC_LIB/src/include" "./src/"
CONFIG += qt

QMAKE_CXXFLAGS += -Wno-deprecated 

MOC_DIR = $$DESTDIR/moc
OBJECTS_DIR = $$DESTDIR/bin
