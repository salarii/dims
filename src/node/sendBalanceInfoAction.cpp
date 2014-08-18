// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sendBalanceInfoAction.h"
#include "common/setResponseVisitor.h"
#include "sendInfoRequestAction.h"

#include "clientFilters.h"

#include "serialize.h"
#include "base58.h"
#include "common/nodeMessages.h"

#include "wallet.h"

using namespace  common;

namespace client
{

CSendBalanceInfoAction::CSendBalanceInfoAction( std::string const _pubKey )
	: m_pubKey( _pubKey )
	, m_actionStatus( common::ActionStatus::Unprepared )
{
}

void
CSendBalanceInfoAction::accept( common::CSetResponseVisitor< NodeResponses > & _visitor )
{
	_visitor.visit( *this );
}

common::CRequest< NodeResponses >*
CSendBalanceInfoAction::execute()
{
	if ( m_actionStatus == ActionStatus::Unprepared )
    {
		m_actionStatus = ActionStatus::InProgress;
        return new CBalanceRequest( m_pubKey );
    }
	else if ( m_actionStatus == ActionStatus::InProgress )
    {
        if ( m_balance )
        {
			CBitcoinAddress address;
			address.SetString( m_pubKey );

			CKeyID keyId;
			address.GetKeyID( keyId );
			typedef std::map< uint256, CCoins >::value_type  MapElement;
			std::vector< CAvailableCoin > availableCoins;
			BOOST_FOREACH( MapElement & coins, *m_balance)
			{
				std::vector< CAvailableCoin > tempCoins = getAvailableCoins( coins.second, keyId, coins.first );
				availableCoins.insert(availableCoins.end(), tempCoins.begin(), tempCoins.end());
			}
			CWallet::getInstance()->setAvailableCoins( keyId, availableCoins );
			m_actionStatus = ActionStatus::Done;
			return 0;
        }
		else if ( m_mediumError )
		{
			// handle medium error somehow
			m_actionStatus = ActionStatus::Done;
			return 0;
		}
        else if ( m_token )
        {
//			return new CInfoRequestContinue( *m_token, new CMediumClassFilter( RequestKind::Balance ) );
        }
    }
    return 0;
}

void
CSendBalanceInfoAction::setMediumError( boost::optional< common::ErrorType::Enum > const & _error )
{
	m_mediumError = _error;
}
void
CSendBalanceInfoAction::setBalance( boost::optional< std::map< uint256, CCoins > > const & _balance )
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

std::vector< CAvailableCoin >
CSendBalanceInfoAction::getAvailableCoins( CCoins const & _coins, uint160 const & _pubId, uint256 const & _hash ) const
{
	std::vector< CAvailableCoin > availableCoins;
	unsigned int valueSum = 0, totalInputSum = 0;
	for (unsigned int i = 0; i < _coins.vout.size(); i++)
	{
		const CTxOut& txout = _coins.vout[i];

		opcodetype opcode;

		std::vector<unsigned char> data;

		CScript::const_iterator pc = txout.scriptPubKey.begin();
		//sanity check
		while( pc != txout.scriptPubKey.end() )
		{
			if (!txout.scriptPubKey.GetOp(pc, opcode, data))
				return std::vector< CAvailableCoin >();
		}
		txnouttype type;

		std::vector< std:: vector<unsigned char> > vSolutions;
		if (Solver(txout.scriptPubKey, type, vSolutions) &&
				(type == TX_PUBKEY || type == TX_PUBKEYHASH))
		{
			std::vector<std::vector<unsigned char> >::iterator it = vSolutions.begin();

			while( it != vSolutions.end() )
			{

				if (
						( ( type == TX_PUBKEY ) && ( _pubId == Hash160( *it ) ) )
					||	( ( type == TX_PUBKEYHASH ) && ( _pubId == uint160( *it ) ) )
					)
				{
					if ( !txout.IsNull() )
						availableCoins.push_back( CAvailableCoin( txout, i, _hash ) );
					break;
				}
				it++;
			}
		}
	}
	return availableCoins;
}


CBalanceRequest::CBalanceRequest( std::string _address )
	: common::CRequest< NodeResponses >( new CMediumClassFilter( RequestKind::Balance ) )
	, m_address( _address )
{
}

void
CBalanceRequest::accept( common::CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

inline
common::CMediumFilter< NodeResponses > *
CBalanceRequest::getMediumFilter() const
{
	return common::CRequest< NodeResponses >::m_mediumFilter;
}

}
