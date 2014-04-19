include $(top_srcdir)/src/Makefile.include

AM_CPPFLAGS += -I$(top_srcdir)/src

bin_PROGRAMS = test_bitcoin

TESTS = test_bitcoin

JSON_TEST_FILES = \
	data/tx_valid.json
#  data/script_valid.json \
#  data/base58_keys_valid.json \
#  data/sig_canonical.json \
#  data/sig_noncanonical.json \
#  data/base58_encode_decode.json \
#  data/base58_keys_invalid.json \
#  data/script_invalid.json \
#  data/tx_invalid.json \
#  data/tx_valid.json

#RAW_TEST_FILES = data/alertTests.raw

BUILT_SOURCES = $(JSON_TEST_FILES:.json=.json.h) $(RAW_TEST_FILES:.raw=.raw.h)

# test_bitcoin binary #
test_bitcoin_CPPFLAGS = $(AM_CPPFLAGS) $(TESTDEFS)
test_bitcoin_LDADD = $(LIBBITCOIN_SERVER) $(LIBBITCOIN_CLI) $(LIBBITCOIN_COMMON) $(LIBLEVELDB) $(LIBMEMENV) \
  $(BOOST_LIBS) $(BOOST_UNIT_TEST_FRAMEWORK_LIB)
if ENABLE_WALLET
test_bitcoin_LDADD += $(LIBBITCOIN_WALLET)
endif
test_bitcoin_LDADD += $(BDB_LIBS)

test_bitcoin_SOURCES = \
  test_ratcoin.cpp \
  uint256_tests.cpp \
  segmentFileStorage_tests.cpp \
  communicationProtocol_tests.cpp \
  $(JSON_TEST_FILES) 

if ENABLE_WALLET
#test_bitcoin_SOURCES += \
#   accounting_tests.cpp \
#   wallet_tests.cpp \
#   rpc_wallet_tests.cpp
endif

nodist_test_bitcoin_SOURCES = $(BUILT_SOURCES)

CLEANFILES = *.gcda *.gcno $(BUILT_SOURCES)
