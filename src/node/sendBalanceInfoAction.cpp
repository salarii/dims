// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sendBalanceInfoAction.h"
#include "setResponseVisitor.h"
#include "sendInfoRequestAction.h"

#include "serialize.h"
#include "support.h"
#include "base58.h"
#include "common/nodeMessages.h"

#include "wallet.h"

using namespace  common;

namespace node
{

CSendBalanceInfoAction::CSendBalanceInfoAction( std::string const _pubKey )
	: m_pubKey( _pubKey )
    , m_status( ActionStatus::Unprepared )
{
}

void
CSendBalanceInfoAction::accept( CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

CRequest*
CSendBalanceInfoAction::execute()
{
    if ( m_status == ActionStatus::Unprepared )
    {
		m_status = ActionStatus::InProgress;
        return new CBalanceRequest( m_pubKey );
    }
    else
    {
        if ( m_balance )
        {
			CBitcoinAddress address;
			address.SetString( m_pubKey );

			CKeyID keyId;
			address.GetKeyID( keyId );
			CWallet::getInstance()->setAvailableCoins( keyId, *m_balance );
            return 0;
        }
        else if ( m_token )
        {
            return new CInfoRequestContinue( *m_token, RequestKind::Balance );
        }
    }
    return 0;
}

void
CSendBalanceInfoAction::setBalance( boost::optional< std::vector< CCoins > > const & _balance )
{
	m_balance = _balance;
}

void
CSendBalanceInfoAction::setInProgressToken( boost::optional< uint256 > const & _token )
{
	m_token = _token;
}

void
CSendBalanceInfoAction::reset()
{
	CAction::reset();

	m_balance.reset();

	m_token.reset();
}

CBalanceRequest::CBalanceRequest( std::string _address )
    : m_address( _address )
{
}

void
CBalanceRequest::serialize( CBufferAsStream & _bufferStream ) const
{
	serializeEnum(_bufferStream, CMainRequestType::BalanceInfoReq );
	_bufferStream << m_address;
}

inline
RequestKind::Enum CBalanceRequest::getKind() const
{
    return RequestKind::Balance;
}

}
