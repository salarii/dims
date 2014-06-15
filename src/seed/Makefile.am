include $(top_srcdir)/src/Makefile.include

AM_CPPFLAGS += -I$(top_srcdir)/src 


bin_PROGRAMS = dims_seed

SEED_H = \
	seedBitcoin.h \
	seedDb.h \
	seedSimplifiedNetworkManager.h
	
SEED_CPP = \
	seedBitcoin.cpp \
	seedMain.cpp \
	seedDb.cpp \
	seedSimplifiedNetworkManager.cpp

BUILT_SOURCES = $(nodist_libmonitor_a_SOURCES)

dns.o: seedDns.c
	gcc -pthread -std=c99 $(CXXFLAGS) seedDns.c -c -o dns.o

# dims_seed binary #
dims_seed_CPPFLAGS = $(AM_CPPFLAGS) -DUSE_IPV6

dims_seed_SOURCES = $(SEED_H) $(SEED_CPP) 

dims_seed_LDADD = $(LIBRATCOIN_TRACKER) $(LIBBITCOIN_SERVER) $(LIBRATCOIN_TRACKER) dns.o 

dims_seed_LDADD += $(LIBBITCOIN_CLI) $(LIBBITCOIN_COMMON) $(LIBLEVELDB) $(LIBMEMENV) $(PROTOBUF_LIBS) \
  $(BOOST_LIBS)  $(BDB_LIBS) 
dims_seed_LDFLAGS = 


.PHONY: FORCE

CLEANFILES = 
