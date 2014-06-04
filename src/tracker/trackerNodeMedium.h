// Copyright (c) 2014 Ratcoin dev-team
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

class CIdentifyRequest;
//I don't know  how to identify which what response is to what  request  yet
class CTrackerNodeMedium : public common::CNodeMedium< TrackerResponses >
{
public:
	CTrackerNodeMedium( common::CSelfNode * _selfNode ):common::CNodeMedium< TrackerResponses >( _selfNode ){};
private:
};

}

#endif // TRACKER_NODE_MEDIUM_H
