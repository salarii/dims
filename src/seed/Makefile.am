include $(top_srcdir)/src/Makefile.include

AM_CPPFLAGS += -I$(top_srcdir)/src 


bin_PROGRAMS = dims_seed

SEED_H = \
	acceptNodeAction.h \
	configureSeedActionHandler.h \
	internalMedium.h \
	pingAction.h \
	processNetwork.h \
	seedBitcoin.h \
	seedDb.h \
	seedNodesManager.h \
	seedNodeMedium.h
	
SEED_CPP = \
	acceptNodeAction.cpp \
	actionHandlerDefinitions.cpp \
	internalMedium.cpp \
	pingAction.cpp \
	processNetwork.cpp \
	seedBitcoin.cpp \
	seedDb.cpp \
	seedMain.cpp \
	seedNodesManager.cpp \
	setResponseVisitor.cpp

BUILT_SOURCES = $(nodist_libmonitor_a_SOURCES)

dns.o: seedDns.c
	gcc -pthread -std=c99 $(CXXFLAGS) seedDns.c -c -o dns.o

# dims_seed binary #
dims_seed_CPPFLAGS = $(AM_CPPFLAGS) -DUSE_IPV6

dims_seed_SOURCES = $(SEED_H) $(SEED_CPP) 

dims_seed_LDADD =  $(LIBDIMS_TRACKER) dns.o 

dims_seed_LDADD += $(LIBBITCOIN_CLI) $(LIBBITCOIN_COMMON)  $(LIBBITCOIN_SERVER) $(LIBLEVELDB) $(LIBMEMENV) $(PROTOBUF_LIBS) \
  $(BOOST_LIBS)  $(BDB_LIBS) 
dims_seed_LDFLAGS = 


.PHONY: FORCE

CLEANFILES = 
