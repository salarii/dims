// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEND_BALANCE_INFO_ACTION_H
#define SEND_BALANCE_INFO_ACTION_H

#include <boost/statechart/state_machine.hpp>

#include "common/action.h"
#include "common/request.h"
#include "configureNodeActionHadler.h"
#include "common/setResponseVisitor.h"

#include  <boost/optional.hpp>
#include "uint256.h"
#include "coins.h"

namespace client
{

struct CGetBalanceInfo;

class CSendBalanceInfoAction : public common::CAction< NodeResponses >, public  boost::statechart::state_machine< CSendBalanceInfoAction, CGetBalanceInfo >
{
public:
	CSendBalanceInfoAction( bool _autoDelete );

	void accept( common::CSetResponseVisitor< NodeResponses > & _visitor );

	common::CRequest< NodeResponses >* getRequest() const;

	void reset();

	void setAddresses( std::vector< std::string > const & _addresses );

	std::vector< std::string > const & getAddresses() const;

	void setRequest( common::CRequest< NodeResponses > * _request );
private:
	common::CRequest< NodeResponses >* m_request;

	std::vector< std::string > m_addresses;
};

struct CBalanceRequest : public common::CRequest< NodeResponses >
{
public:
	CBalanceRequest( std::string _address );
	common::CMediumFilter< NodeResponses > * getMediumFilter() const;
	void accept( common::CMedium< NodeResponses > * _medium ) const;
    std::string const m_address;

};

}

#endif // SEND_BALANCE_INFO_ACTION_H
