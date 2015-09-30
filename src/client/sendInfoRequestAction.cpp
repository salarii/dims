// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sendInfoRequestAction.h"

#include "serialize.h"
#include "common/support.h"
#include "common/setResponseVisitor.h"
#include "filters.h"

#include <boost/assign/list_of.hpp>

namespace client
{

std::vector< TrackerInfo::Enum > const TrackerDescription = boost::assign::list_of< TrackerInfo::Enum >( TrackerInfo::Ip);//.convert_to_container<std::vector< TrackerInfo::Enum > >();

CSendInfoRequestAction::CSendInfoRequestAction( NetworkInfo::Enum const _networkInfo )
{
}

void
CSendInfoRequestAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}
