// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEED_NODE_MEDIUM_H
#define SEED_NODE_MEDIUM_H

#include "common/nodeMedium.h"

#include "configureSeedActionHandler.h"

namespace seed
{


class CIdentifyRequest;

class CSeedNodeMedium : public common::CNodeMedium< SeedResponses >
{
public:
	CSeedNodeMedium( common::CSelfNode * _selfNode ):common::CNodeMedium< SeedResponses >( _selfNode ){};
private:
};

}


#endif // SEED_NODE_MEDIUM_H
