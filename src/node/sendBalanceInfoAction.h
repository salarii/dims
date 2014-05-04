// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEND_BALANCE_INFO_ACTION_H
#define SEND_BALANCE_INFO_ACTION_H

#include "action.h"
#include "request.h"

#include  <boost/optional.hpp>
#include "uint256.h"
#include "coins.h"

namespace node
{

class CSendBalanceInfoAction : public CAction
{
public:
	CSendBalanceInfoAction( std::string const _pubKey );

	void accept( CSetResponseVisitor & _visitor );

	CRequest* execute();

	void setBalance( boost::optional< std::map< uint256, CCoins > > const & _balance );

	void setInProgressToken( boost::optional< uint256 > const & _token );

	void reset();
private:
	std::vector< CAvailableCoin > getAvailableCoins( CCoins const & _coins, uint160 const & _pubId, uint256 const & _hash ) const;
private:
	CRequest* m_request;

	std::string const m_pubKey;

	boost::optional< std::map< uint256, CCoins > > m_balance;

	boost::optional< uint256 > m_token;
};

struct CBalanceRequest : public CRequest
{
public:
    CBalanceRequest( std::string _address );
    void serialize( CBufferAsStream & _bufferStream ) const;
    RequestKind::Enum getKind() const;

    std::string const m_address;

};

}

#endif // SEND_BALANCE_INFO_ACTION_H
