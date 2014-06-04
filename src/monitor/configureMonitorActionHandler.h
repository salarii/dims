#ifndef CONFIGURE_MONITOR_ACTION_HANDLER_H
#define CONFIGURE_MONITOR_ACTION_HANDLER_H

#include <boost/mpl/list.hpp>
#include <boost/variant/variant.hpp>
#include "common/commonResponses.h"


namespace monitor
{

struct CDummyResponse{};

typedef boost::mpl::list< common::CMediumException, CDummyResponse, common::CIdentificationResult, common::CContinueResult > MonitorResponseList;

typedef boost::make_variant_over< MonitorResponseList >::type MonitorResponses;

}


#endif // CONFIGURE_MONITOR_ACTION_HANDLER_H
