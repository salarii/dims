include $(top_srcdir)/src/Makefile.include

AM_CPPFLAGS += -I$(top_srcdir)/src 


bin_PROGRAMS = dims_monitor

noinst_LIBRARIES = libmonitor.a

MONITOR_H = \
	admitTrackerAction.h \
	admitTransactionsBundle.h \
	clientRequestsManager.h \
	connectNodeAction.h \
	filters.h \
	init.h \
	internalMediumProvider.h \
	internalOperationsMedium.h \
	monitorController.h \
	monitorNodeMedium.h \
	monitorRequests.h \
	noMediumHandling.h \
	pingAction.h \
	processNetwork.h \
	rankingDatabase.h \
	reputationTracer.h \
	server.h \
	updateDataAction.h

	
MONITOR_CPP = \
	actionHandlerDefinitions.cpp \
	admitTrackerAction.cpp \
	admitTransactionsBundle.cpp \
	clientRequestsManager.cpp \
	connectNodeAction.cpp \
	init.cpp \
	internalMediumProvider.cpp \
	internalOperationsMedium.cpp \
	monitor.cpp \
	monitorController.cpp \
	monitorNodeMedium.cpp \
	monitorRequests.cpp \
	noMediumHandling.cpp \
	pingAction.cpp \
	processNetwork.cpp \
	rankingDatabase.cpp \
	reputationTracer.cpp \
	server.cpp \
	setResponseVisitor.cpp \
	updateDataAction.cpp

libmonitor_a_CPPFLAGS = $(AM_CPPFLAGS) $(QT_INCLUDES) 
libmonitor_a_SOURCES = $(MONITOR_H) $(MONITOR_CPP)

nodist_libmonitor_a_SOURCES = $(MONITOR_H) $(MONITOR_CPP)
  

BUILT_SOURCES = $(nodist_libmonitor_a_SOURCES)

# dims_monitor binary #
dims_monitor_CPPFLAGS = $(AM_CPPFLAGS)

dims_monitor_SOURCES = monitor.cpp

dims_monitor_LDADD =  libmonitor.a $(LIBDIMS_TRACKER) $(LIBBITCOIN_SERVER) $(LIBDIMS_TRACKER) 

dims_monitor_LDADD += $(LIBBITCOIN_CLI) $(LIBBITCOIN_COMMON) $(LIBLEVELDB) $(LIBMEMENV) $(PROTOBUF_LIBS) \
  $(BOOST_LIBS)  $(BDB_LIBS) 
dims_monitor_LDFLAGS = $(QT_LDFLAGS)


.PHONY: FORCE

CLEANFILES = 
