// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_NODE_MEDIUM_H
#define TRACKER_NODE_MEDIUM_H

#include "common/nodeMedium.h"
#include "common/communicationProtocol.h"

#include <boost/variant.hpp>

namespace tracker
{

typedef boost::variant< common::CIdentifyMessage > ProtocolMessage;

class CGetSynchronizationInfoRequest;

class CTrackerNodeMedium : public common::CNodeMedium< common::CTrackerBaseMedium >
{
public:
	CTrackerNodeMedium( common::CSelfNode * _selfNode ):common::CNodeMedium< common::CTrackerBaseMedium >( _selfNode ){};

	void add( CGetSynchronizationInfoRequest const * _request );

	void add( CTransactionsPropagationRequest const * _request );

	void add( CTransactionsStatusRequest const * _request );

	void add( CPassMessageRequest const * _request );

	void add( CDeliverInfoRequest const * _request );

	void add( CAskForRegistrationRequest const * _request );

	void add( CRegisterProofRequest const * _request );

	void add( CTransactionAsClientRequest const * _request );
private:
};

}

#endif // TRACKER_NODE_MEDIUM_H
