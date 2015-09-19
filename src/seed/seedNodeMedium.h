// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEED_NODE_MEDIUM_H
#define SEED_NODE_MEDIUM_H

#include "common/nodeMedium.h"

namespace seed
{

class CSeedNodeMedium : public common::CNodeMedium
{
public:
	CSeedNodeMedium( common::CSelfNode * _selfNode ):common::CNodeMedium( _selfNode ){};
private:
};

}


#endif // SEED_NODE_MEDIUM_H
