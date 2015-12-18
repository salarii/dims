QT       += core network gui widgets

TARGET = app

SOURCES += main.cpp \
    testApplicaionWidgets.cpp

HEADERS  += testApplicaionWidgets.h

LIBS += -L. -ldimsapp -lboost_thread  -lboost_system -lboost_filesystem -lboost_program_options -lssl -lcrypto 

INCLUDEPATH = . ..