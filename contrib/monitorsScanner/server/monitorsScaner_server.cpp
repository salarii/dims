// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gen-cpp/MonitorsScaner.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "util.h"

#include "informationProvider.h"
#include "networksParameters.h"

#include "common/actionHandler.h"
#include "common/periodicActionExecutor.h"
#include "common/dimsParams.h"

#include "client/settingsConnectionProvider.h"
#include "client/connectAction.h"
#include "client/errorMediumProvider.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::monitorsScaner;


class MonitorsScanerHandler : virtual public MonitorsScanerIf
{
 public:
  MonitorsScanerHandler()
  {}

  void getInfo(Data& _return, const InfoRequest& infoRequest)
  {
	CInforamtionProvider::getInstance()->getInfo( _return, infoRequest );
  }

};

void
init( boost::thread_group & _threadGroup )
{
	common::SelectDimsParams( CNetworkParams::TESTNET );

	common::CPeriodicActionExecutor * periodicActionExecutor
			= common::CPeriodicActionExecutor::getInstance();

	_threadGroup.create_thread(boost::bind(&common::CPeriodicActionExecutor::processingLoop, periodicActionExecutor ));

	_threadGroup.create_thread(boost::bind(&common::CActionHandler::loop, common::CActionHandler::getInstance()));

	common::CActionHandler::getInstance()->addConnectionProvider( client::CSettingsConnectionProvider::getInstance() );

	common::CActionHandler::getInstance()->addConnectionProvider( client::CTrackerLocalRanking::getInstance() );

	common::CActionHandler::getInstance()->addConnectionProvider( client::CErrorMediumProvider::getInstance() );

	client::CConnectAction * connectAction =  new client::CConnectAction( false );

	connectAction->m_connected.connect( boost::bind( &CInforamtionProvider::reloadData, CInforamtionProvider::getInstance() ) );

	common::CPeriodicActionExecutor::getInstance()->addAction( connectAction, 60000 );
}

int main(int argc, char **argv)
{
	int port = 9090;
	ParseParameters(argc, argv);

  shared_ptr<MonitorsScanerHandler> handler(new MonitorsScanerHandler());
  shared_ptr<TProcessor> processor(new MonitorsScanerProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  boost::thread_group threadGroup;

  init( threadGroup );

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();

	threadGroup.join_all();

  return 0;
}

