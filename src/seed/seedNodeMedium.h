#ifndef SEED_NODE_MEDIUM_H
#define SEED_NODE_MEDIUM_H

#include "common/nodeMedium.h"

#include "configureSeedActionHandler.h"

namespace seed
{


class CIdentifyRequest;

class CTrackerNodeMedium : public common::CNodeMedium< SeedResponses >
{
public:
	CTrackerNodeMedium( common::CSelfNode * _selfNode ):common::CNodeMedium< SeedResponses >( _selfNode ){};
private:
};

}


#endif // SEED_NODE_MEDIUM_H
