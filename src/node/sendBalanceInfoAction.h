// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEND_BALANCE_INFO_ACTION_H
#define SEND_BALANCE_INFO_ACTION_H

#include "common/action.h"
#include "common/request.h"
#include "configureNodeActionHadler.h"
#include "common/setResponseVisitor.h"

#include  <boost/optional.hpp>
#include "uint256.h"
#include "coins.h"

namespace client
{

class CSendBalanceInfoAction : public common::CAction< NodeResponses >
{
public:
	CSendBalanceInfoAction( std::string const _pubKey );

	void accept( common::CSetResponseVisitor< NodeResponses > & _visitor );

	common::CRequest< NodeResponses >* execute();

	void setBalance( boost::optional< std::map< uint256, CCoins > > const & _balance );

	void setInProgressToken( boost::optional< uint256 > const & _token );

	void setMediumError( boost::optional< common::ErrorType::Enum > const & _error );

	void reset();
private:
	std::vector< CAvailableCoin > getAvailableCoins( CCoins const & _coins, uint160 const & _pubId, uint256 const & _hash ) const;
private:
	common::CRequest< NodeResponses >* m_request;

	std::string const m_pubKey;

	boost::optional< std::map< uint256, CCoins > > m_balance;

	boost::optional< uint256 > m_token;

	boost::optional< common::ErrorType::Enum > m_mediumError;

	common::ActionStatus::Enum m_actionStatus;
};

struct CBalanceRequest : public common::CRequest< NodeResponses >
{
public:
	CBalanceRequest( std::string _address );
	int getMediumFilter() const;
	void accept( common::CMedium< NodeResponses > * _medium ) const;
    std::string const m_address;

};

}

#endif // SEND_BALANCE_INFO_ACTION_H
