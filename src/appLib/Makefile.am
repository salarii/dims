include $(top_srcdir)/src/Makefile.include

AM_CPPFLAGS += -I$(top_srcdir)/src 
  
bin_PROGRAMS = dims_appTest
noinst_LIBRARIES = libdimsapp.a


APP_MOC_CPP = \
  moc_appClient.cpp \
  moc_testApplicaionWidgets.cpp

APP_QT_H = \
  ../common/dimsParams.h \
  ../common/commonStruct.h \
  ../addrman.h \
  ../allocators.h \
  ../base58.h \
  ../chainparams.h \
  ../core.h \
  ../hash.h \
  ../key.h \
  ../netbase.h \
  ../protocol.h \
  ../script.h \
  ../serialize.h \
  ../version.h \
  ../util.h \
  appClient.h \
  hardwareInfo.h \
  messageType.h \
  paymentData.h\
  paymentProcessing.h \
  testApplicaionWidgets.h
  
APP_QT_CPP = \
  ../common/dimsParams.cpp \
  ../addrman.cpp \
  ../allocators.cpp \
  ../chainparams.cpp \
  ../core.cpp \
  ../hash.cpp \
  ../key.cpp \
  ../netbase.cpp \
  ../protocol.cpp \
  ../script.cpp \
  ../version.cpp \
  ../util.cpp \
  appClient.cpp \
  hardwareInfo.cpp \
  paymentProcessing.cpp \
  testApplicaionWidgets.cpp	
  
libdimsapp_a_CPPFLAGS = $(AM_CPPFLAGS) $(QT_INCLUDES) \
   $(QT_DBUS_INCLUDES)
   
libdimsapp_a_SOURCES = $(APP_QT_CPP) $(APP_QT_H)  

nodist_libdimsapp_a_SOURCES = $(APP_MOC_CPP)

BUILT_SOURCES = $(nodist_libdimsapp_a_SOURCES)


dims_appTest_CPPFLAGS = $(AM_CPPFLAGS) $(QT_INCLUDES) 

dims_appTest_SOURCES = main.cpp

dims_appTest_LDADD = libdimsapp.a 

dims_appTest_LDADD += $(LIBLEVELDB) $(LIBMEMENV) $(PROTOBUF_LIBS) $(BOOST_LIBS) $(QT_LIBS) $(QT_DBUS_LIBS)  

dims_appTest_LDFLAGS = $(QT_LDFLAGS)

.PHONY: FORCE
.SECONDARY: $(QT_QM)

bitcoinstrings.cpp: FORCE
	$(MAKE) -C $(top_srcdir)/src 

CLEANFILES = $(BUILT_SOURCES) *.gcda *.gcno
