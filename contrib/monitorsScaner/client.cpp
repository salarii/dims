#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "./gen-cpp/MonitorsScaner.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace monitorsScaner;

int main(int argc, char** argv) {
  boost::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
  boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  MonitorsScanerClient client(protocol);

  try {
    transport->open();


    Data returnData;

    InfoRequest infoRequest;

    infoRequest.networkType = NetworkType::MAIN;

    infoRequest.info = Info::TRACKERS_INFO;

    infoRequest.key = "15sxZ93LFygg2Rvht6FSTtfxaTcGNSpSc5";

    client.getInfo(returnData, infoRequest);


    transport->close();
  } catch (TException &tx) {
    printf("ERROR: %s\n", tx.what());
  }

}
