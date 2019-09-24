release {
	DESTDIR = build/release
}

debug {
	DESTDIR = build/debug	
}

HEADERS = AI_RAW_VALUE.h \
	AI_VALUE.h \
	BOARD_INFO_WIDGET.h \
	CAL_VALUE.h \
	DO_INDICATOR.h \
	LOGIC_INFO.h \
	MAIN_WINDOW.h \
	PMIC_INDICATOR.h \
	RAW_BOARD_INFO.h

SOURCES = AI_RAW_VALUE.cpp \
	BOARD_INFO_WIDGET.cpp \
	CAL_VALUE.cpp \
	DO_INDICATOR.cpp \
	LOGIC_INFO.cpp \
	main.cpp \
	MAIN_WINDOW.cpp \
	PMIC_INDICATOR.cpp \
	RAW_BOARD_INFO.cpp \
	AI_VALUE.cpp

LIBS += -L../HVAC_LIB/bin -lHVAC_LIB
INCLUDEPATH += "../HVAC_LIB/src/include"
CONFIG += qt


MOC_DIR = $$DESTDIR/moc
OBJECTS_DIR = $$DESTDIR/bin
