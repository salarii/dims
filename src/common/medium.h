// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef	MEDIUM_H
#define MEDIUM_H

#include "common/responses.h"

#include "visitorConfigurationUtilities.h"

namespace common
{

class CSendIdentifyDataRequest;
class CAckRequest;
class CTimeEventRequest;
class CScheduleActionRequest;
class CInfoAskRequest;
class CConnectToNodeRequest;
class CAskForTransactionsRequest;
class CBalanceRequest;
class CSendMessageRequest;
class CSetBloomFilterRequest;
}

namespace tracker
{
class CValidateTransactionsRequest;
class CConnectToTrackerRequest;
class CTransactionsStatusRequest;
class CTransactionsPropagationRequest;
class CPassMessageRequest;
class CDeliverInfoRequest;
class CGetBalanceRequest;
class CAskForRegistrationRequest;

}

namespace client
{
struct CDnsInfoRequest;
struct CErrorForAppPaymentProcessing;
struct CProofTransactionAndStatusRequest;
}

namespace monitor
{
class CConnectToNodeRequest;
class CInfoRequest;
}

namespace common
{

class CAction;

class CRequest;

class CMedium
{
public:
	virtual bool serviced() const = 0;
	virtual bool flush() = 0;
	virtual void prepareMedium(){};
	virtual void deleteRequest( CRequest const* _request ){};// needed in some cases
	virtual bool getResponseAndClear( std::multimap< CRequest const*, DimsResponse > & _requestResponse) = 0;// the order of  elements with the same key is important, I have read somewhere that in this c++ standard this is not guaranteed but "true in practice":  is  such assertion good  enough??
	virtual bool getDirectActionResponseAndClear( CAction const * _action, std::list< DimsResponse > & _responses ){ return false; }
	virtual void deleteAction( CAction const * _action ){};

	virtual void add( common::CTimeEventRequest const * _request ){};
	virtual void add( common::CSendMessageRequest const * _request ){};
	virtual void add( common::CAckRequest const * _request ){}
	virtual void add( common::CSetBloomFilterRequest const * _request ){};
	virtual void add( common::CSendIdentifyDataRequest const * _request ){};
	virtual void add( common::CScheduleActionRequest const * _request ){};
	virtual void add( common::CInfoAskRequest const * _request ){};
	virtual void add( common::CBalanceRequest const * _request ){};
	virtual void add( common::CAskForTransactionsRequest const * _request ){};
	virtual void add( common::CConnectToNodeRequest const * _request ){};
	virtual void add( client::CDnsInfoRequest const * _request ){};
	virtual void add( client::CErrorForAppPaymentProcessing const * _request ){};
	virtual void add( client::CProofTransactionAndStatusRequest const * _request ){};
	virtual void add( tracker::CGetBalanceRequest const * _request ){};
	virtual void add( tracker::CValidateTransactionsRequest const * _request ){};
	virtual void add( tracker::CConnectToTrackerRequest const * _request ){};
	virtual void add( tracker::CTransactionsStatusRequest const * _request ){};
	virtual void add( tracker::CTransactionsPropagationRequest const * _request ){};
	virtual void add( tracker::CPassMessageRequest const * _request ){};
	virtual void add( tracker::CDeliverInfoRequest const * _request ){};
	virtual void add( tracker::CAskForRegistrationRequest const * _request ){};
	virtual void add( monitor::CInfoRequest const * _request ){};
	virtual void add( monitor::CConnectToNodeRequest const * _request ){};

	void registerDeleteHook( boost::signals2::slot< void () > const & _deleteHook )
	{
		m_deleteHook.connect( _deleteHook );
	}

	virtual ~CMedium()
	{
		m_deleteHook();
	};

	boost::signals2::signal<void ()> m_deleteHook;
};

}
#endif // MEDIUM_H
