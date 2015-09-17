// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEND_BALANCE_INFO_ACTION_H
#define SEND_BALANCE_INFO_ACTION_H

#include <boost/statechart/state_machine.hpp>

#include "common/action.h"
#include "common/request.h"
#include "configureClientActionHadler.h"
#include "common/setResponseVisitor.h"

#include  <boost/optional.hpp>
#include "uint256.h"
#include "coins.h"

namespace client
{

struct CGetBalanceInfo;

class CSendBalanceInfoAction : public common::CAction< common::CClientTypes >, public  boost::statechart::state_machine< CSendBalanceInfoAction, CGetBalanceInfo >
{
public:
	CSendBalanceInfoAction( bool _autoDelete );

	void accept( common::CSetResponseVisitor< common::CClientTypes > & _visitor );

	void reset();

	void setAddresses( std::vector< std::string > const & _addresses );

	std::vector< std::string > const & getAddresses() const;
private:
	std::vector< std::string > m_addresses;
};

}

#endif // SEND_BALANCE_INFO_ACTION_H
