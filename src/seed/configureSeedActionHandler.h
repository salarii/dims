#ifndef CONFIGURE_SEED_ACTION_HANDLER_H
#define CONFIGURE_SEED_ACTION_HANDLER_H

#include <boost/mpl/list.hpp>
#include <boost/variant/variant.hpp>
#include "common/commonResponses.h"
#include "common/commonEvents.h"

namespace seed
{

struct CDummyResponse{};

typedef boost::mpl::list< common::CPingPongResult, common::CMediumException, common::CIdentificationResult, common::CConnectedNode, common::CAckResult, common::CEndEvent, common::CTimeEvent > SeedResponseList;

typedef boost::make_variant_over< SeedResponseList >::type SeedResponses;

}


#endif // CONFIGURE_SEED_ACTION_HANDLER_H
