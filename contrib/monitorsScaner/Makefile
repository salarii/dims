GEN_CPP_SRC := MonitorsScaner.cpp monitorsScaner_constants.cpp monitorsScaner_types.cpp
SERVER_SRC := monitorsScaner_server.cpp informationProvider.cpp
GEN_OBJ := $(patsubst %.cpp,gen-cpp/%.o, $(GEN_CPP_SRC))
SERVER_OBJ := $(patsubst %.cpp,server/%.o, $(SERVER_SRC))

CXX := g++

THRIFT_DIR := /usr/local/include/thrift
BOOST_DIR := /usr/local/include
GEN_DIR := gen-cpp

LIB_DIR := /usr/local/lib

LIBS := -L$(LIB_DIR) -lboost_thread -lboost_system -lboost_filesystem

INC := -I$(THRIFT_DIR) -I$(BOOST_DIR) -I$(GEN_DIR)

.PHONY: all clean

all: server_bin

%.o: %.cpp
	$(CXX) -Wall -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H   $(INC) $(LIBS) -c $< -o $@
server_bin:  $(GEN_OBJ) $(SERVER_OBJ)
	$(CXX) $^ -o $@ -L/usr/local/lib -lthrift $(LIBS)
clean:
	$(RM) *.o gen-cpp/*.o server/*.o server_bin