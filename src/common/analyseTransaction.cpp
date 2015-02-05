// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "core.h"
#include "coins.h"
#include "analyseTransaction.h"

namespace common
{

bool
findOutputInTransaction( CTransaction const & _tx, CKeyID const & _findId, CTxOut & _txout, unsigned int & _id )
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
						_txout = txout;
						_id = i;
						return true;
					}
				}
				else
				{
					if ( _findId == uint160( *it ) )
					{
						_txout = txout;
						_id = i;
						return true;
					}
				}
				it++;
			}
		}

	}
	return false;
}

std::vector< CAvailableCoin >
getAvailableCoins( CCoins const & _coins, uint160 const & _pubId, uint256 const & _hash )
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

bool
getRealCountOfCoinsSpend( CTransaction const & _tx, CKeyID const & _excaption, unsigned int & _outCount )
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
				keys.push_back( CPubKey( data ).GetID() );
			}
		}
	}

	std::map< CKeyID, unsigned int > balances;

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
					balances.insert( std::make_pair( Hash160( *it ), txout.nValue ) );
				}
				else
				{
					balances.insert( std::make_pair( uint160( *it ), txout.nValue ) );
				}
				it++;
			}
		}
	}

	balances.erase( _excaption );
	BOOST_FOREACH( CKeyID const & keyId, keys )
	{
		balances.erase( keyId );
	}

	_outCount = 0;

	BOOST_FOREACH( PAIRTYPE( CKeyID, unsigned int ) const & output, balances )
	{
		_outCount += output.second;
	}
	return true;
}
}
