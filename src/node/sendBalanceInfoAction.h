// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEND_BALANCE_INFO_ACTION_H
#define SEND_BALANCE_INFO_ACTION_H

#include "action.h"
#include "tracker/validationManager.h"
#include "requestHandler.h"

namespace node
{

class CSendBalanceInfoAction : public CAction
{
public:
	CSendBalanceInfoAction( std::string const _pubKey );

	void accept( CSetResponseVisitor & _visitor );

	CRequest* execute();

	void setBalance( boost::optional< std::string > const & _balance );

	void setInProgressToken( boost::optional< uint256 > const & _token );
private:
	CRequest* m_request;

	std::string const m_pubKey;

	boost::optional< std::string > m_balance;

	boost::optional< uint256 > m_token;
};

}

#endif // SEND_BALANCE_INFO_ACTION_H