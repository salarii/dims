// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "core.h"
#include "coins.h"
#include "wallet.h"
#include "common/analyseTransaction.h"
#include "common/authenticationProvider.h"

namespace common
{

bool
getTransactionInputs( CTransaction const & _tx, std::vector< CKeyID > & _inputs )
{
// first which apear gets all because  it is a little bit tricky to find out real contribution of each address
	for (unsigned int i = 0; i < _tx.vin.size(); i++)
	{
		const CTxIn& txin = _tx.vin[i];

		CScript::const_iterator pc = txin.scriptSig.begin();

		opcodetype opcode;

		std::vector<unsigned char> data;

		while( pc < txin.scriptSig.end() )
		{
			if (!txin.scriptSig.GetOp(pc, opcode, data))
				return false;

			if ( data.size() == 33 || data.size() == 65 )
			{
				_inputs.push_back( CPubKey( data ).GetID() );
			}
		}
	}
	return true;
}

bool
findOutputInTransaction( CTransaction const & _tx, CKeyID const & _findId, std::vector< CTxOut > & _txouts, std::vector< unsigned int > & _ids )
{
	for (unsigned int i = 0; i < _tx.vout.size(); i++)
	{
		const CTxOut& txout = _tx.vout[i];

		opcodetype opcode;

		std::vector<unsigned char> data;

		CScript::const_iterator pc = txout.scriptPubKey.begin();
	//sanity check
		while( pc != txout.scriptPubKey.end() )
		{
			if (!txout.scriptPubKey.GetOp(pc, opcode, data))
				return false;
		}
		txnouttype type;

		std::vector< std:: vector<unsigned char> > vSolutions;
		if (Solver(txout.scriptPubKey, type, vSolutions) &&
			(type == TX_PUBKEY || type == TX_PUBKEYHASH))
		{
			std::vector<std::vector<unsigned char> >::iterator it = vSolutions.begin();

			while( it != vSolutions.end() )
			{
				if ( type == TX_PUBKEY )
				{
					// impossible  to be here ??
					if ( _findId == Hash160( *it ) )
					{
						_txouts.push_back( txout );
						_ids.push_back( i );
					}
				}
				else
				{
					if ( _findId == uint160( *it ) )
					{
						_txouts.push_back( txout );
						_ids.push_back( i );
					}
				}
				it++;
			}
		}

	}
	return !_txouts.empty();
}

std::vector< CAvailableCoin >
getAvailableCoins( CCoins const & _coins, uint160 const & _pubId, uint256 const & _hash )
{
	std::vector< CAvailableCoin > availableCoins;
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


bool
findKeyInInputs( CTransaction const & _tx, CKeyID const & _keyId )
{

	std::list< CKeyID > keys;
	for (unsigned int i = 0; i < _tx.vin.size(); i++)
	{
		const CTxIn& txin = _tx.vin[i];

		CScript::const_iterator pc = txin.scriptSig.begin();

		opcodetype opcode;

		std::vector<unsigned char> data;

		while( pc < txin.scriptSig.end() )
		{
			if (!txin.scriptSig.GetOp(pc, opcode, data))
				return false;

			if ( data.size() == 33 || data.size() == 65 )
			{
				if ( _keyId == CPubKey( data ).GetID() )
					return true;
			}
		}
	}

	return false;
}

void
findSelfCoinsAndAddToWallet( CTransaction const & _tx )
{
	CKeyID keyId = common::CAuthenticationProvider::getInstance()->getMyKey().GetID();

	std::vector< CAvailableCoin > availableCoins
			= common::getAvailableCoins(
				_tx
				, keyId
				, _tx.GetHash() );

	CWallet::getInstance()->addAvailableCoins( keyId, availableCoins );
}

}
