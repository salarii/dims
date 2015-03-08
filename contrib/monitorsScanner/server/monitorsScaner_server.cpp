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

#include "common/actionHandler.h"
#include "common/periodicActionExecutor.h"

#include "client/settingsConnectionProvider.h"
#include "client/configureClientActionHadler.h"
#include "client/connectAction.h"

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
	m_informationProvider.getInfo( _return, infoRequest );
  }

CInforamtionProvider m_informationProvider;
};

/*
template<>
unsigned int const common::CActionHandler< common::CClientTypes >::m_sleepTime = 100;
template<>
common::CActionHandler< common::CClientTypes > * common::CActionHandler< common::CClientTypes >::ms_instance = NULL;


template<>
common::CPeriodicActionExecutor< common::CClientTypes > * common::CPeriodicActionExecutor< common::CClientTypes >::ms_instance = NULL;

template<>
unsigned int const common::CPeriodicActionExecutor< common::CClientTypes >::m_sleepTime = 100;
*/
void
init( boost::thread_group & _threadGroup )
{
	common::SelectRatcoinParamsFromCommandLine();

	common::CPeriodicActionExecutor< common::CClientTypes > * periodicActionExecutor
			= common::CPeriodicActionExecutor< common::CClientTypes >::getInstance();

	_threadGroup.create_thread(boost::bind(&common::CPeriodicActionExecutor< common::CClientTypes >::processingLoop, periodicActionExecutor ));

	_threadGroup.create_thread(boost::bind(&common::CActionHandler< common::CClientTypes >::loop, common::CActionHandler< common::CClientTypes >::getInstance()));

	common::CActionHandler< common::CClientTypes >::getInstance()->addConnectionProvider( client::CSettingsConnectionProvider::getInstance() );

	common::CActionHandler< common::CClientTypes >::getInstance()->addConnectionProvider( client::CTrackerLocalRanking::getInstance() );

	common::CPeriodicActionExecutor< common::CClientTypes >::getInstance()->addAction( new client::CConnectAction( false ), 60000 );
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

  threadGroup.create_thread( boost::bind( &CInforamtionProvider::reloadThread, &handler->m_informationProvider ) );

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();

	threadGroup.join_all();

  return 0;
}

