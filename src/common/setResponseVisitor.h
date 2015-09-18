// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SET_RESPONSE_VISITOR_H
#define SET_RESPONSE_VISITOR_H

#include <boost/variant.hpp> 
#include "common/responses.h"

namespace tracker
{

class CGetBalanceAction;

class CValidateTransactionsAction;

class CConnectNodeAction;

class CTrackOriginAddressAction;

class CSynchronizationAction;

class CProvideInfoAction;

class CPingAction;

class CRegisterAction;

class CRecognizeNetworkAction;

class CPassTransactionAction;

class CConnectNetworkAction;
}

namespace client
{

class CSendBalanceInfoAction;

class CSendTransactionAction;

class CConnectAction;

class CPayLocalApplicationAction;

class CSendInfoRequestAction;
}

namespace monitor
{
class CConnectNodeAction;

class CUpdateDataAction;

class CAdmitTrackerAction;

class CAdmitTransactionBundle;

class CPingAction;

class CRecognizeNetworkAction;

class CTrackOriginAddressAction;

class CProvideInfoAction;

class CCopyTransactionStorageAction;

class CSynchronizationAction;

class CEnterNetworkAction;
}

namespace seed
{
class CAcceptNodeAction;

class CPingAction;
}

namespace common
{

class CAction;

class CSetResponseVisitor
{
public:
	CSetResponseVisitor( common::DimsResponse const & _response ):m_responses(_response)
	{}

	~CSetResponseVisitor()
	{}

	void visit( common::CAction & _action ){};

	void visit( tracker::CGetBalanceAction & _action );

	void visit( tracker::CValidateTransactionsAction & _action );

	void visit( tracker::CConnectNodeAction & _action );

	void visit( tracker::CTrackOriginAddressAction & _action );

	void visit( tracker::CSynchronizationAction & _action );

	void visit( tracker::CProvideInfoAction & _action );

	void visit( tracker::CPingAction & _action );

	void visit( tracker::CRegisterAction & _action );

	void visit( tracker::CRecognizeNetworkAction & _action );

	void visit( tracker::CPassTransactionAction & _action );

	void visit( tracker::CConnectNetworkAction & _action );

	void visit( client::CSendTransactionAction & _action );

	void visit( client::CConnectAction & _action );

	void visit( client::CSendBalanceInfoAction & _action );

	void visit( client::CPayLocalApplicationAction & _action );

	void visit( client::CSendInfoRequestAction & _action );

	void visit( monitor::CConnectNodeAction & _action );

	void visit( monitor::CUpdateDataAction & _action );

	void visit( monitor::CAdmitTrackerAction & _action );

	void visit( monitor::CAdmitTransactionBundle & _action );

	void visit( monitor::CPingAction & _action );

	void visit( monitor::CRecognizeNetworkAction & _action );

	void visit( monitor::CTrackOriginAddressAction & _action );

	void visit( monitor::CProvideInfoAction & _action );

	void visit( monitor::CEnterNetworkAction & _action );

	void visit( monitor::CCopyTransactionStorageAction & _action );

	void visit( monitor::CSynchronizationAction & _action );

	void visit( seed::CAcceptNodeAction & _action );

	void visit( seed::CPingAction & _action );
protected:
	common::DimsResponse m_responses;
};


}

#endif
