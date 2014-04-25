// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "tracker/originAddressScaner.h"
#include "tracker/transactionRecordManager.h"

#include "chainparams.h"
#include "base58.h"
#include "core.h"

#include "hash.h"

#include <vector>

namespace tracker
{

COriginAddressScaner * COriginAddressScaner::ms_instance = NULL;
	
COriginAddressScaner*
COriginAddressScaner::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new COriginAddressScaner();
	};
	return ms_instance;
}

COriginAddressScaner::COriginAddressScaner()
{
}

void
COriginAddressScaner::saveBalanceToDatabase()
{
}

void
COriginAddressScaner::createCoinBaseTransaction()
{
}

void
COriginAddressScaner::getHeightOfLastScanedBlock()
{
}

void
COriginAddressScaner::addTransaction( long long const _indexHeight, CTransaction const&  _tx)
{
	boost::lock_guard<boost::mutex> lock(m_lock);

	m_transactionToProcess.insert( std::make_pair(_indexHeight, _tx) );
}

void
COriginAddressScaner::Thread()
{
	while(1)
	{
		{
			boost::lock_guard<boost::mutex> lock(m_lock);

			if ( m_transactionToProcess.size() )
			{
				std::map< long long, CTransaction >::iterator it = m_transactionToProcess.begin();
				while( it != m_transactionToProcess.end() )
				{
					createBaseTransaction( it->second );
					it++;
				}
				m_transactionToProcess.clear();
			}
		}
		boost::this_thread::interruption_point();
	}

}

void COriginAddressScaner::createBaseTransaction(CTransaction const &  _tx)
{
	unsigned int valueSum = 0, totalInputSum = 0;
	for (unsigned int i = 0; i < _tx.vout.size(); i++)
	{
		const CTxOut& txout = _tx.vout[i];

		opcodetype opcode;

		std::vector<unsigned char> data;

		CScript::const_iterator pc = txout.scriptPubKey.begin();
/*
		while( pc != txout.scriptPubKey.end() )
		{
			if (!txout.scriptPubKey.GetOp(pc, opcode, data))
				return;

			pc++;
		}*/
		txnouttype type;

		std::vector< std:: vector<unsigned char> > vSolutions;
		if (Solver(txout.scriptPubKey, type, vSolutions) &&
			(type == TX_PUBKEY || type == TX_PUBKEYHASH))
		{
			std::vector<std::vector<unsigned char> >::iterator it = vSolutions.begin();

			while( it != vSolutions.end() )
			{
				 uint160 originId( Params().getOriginAddressKeyId() );
				if ( type == TX_PUBKEY )
				{
					if ( originId == Hash160( *it ) )
						valueSum += txout.nValue;
				}
				else
				{
					if ( originId == uint160( ( *it ) ) )
						valueSum += txout.nValue;
				}
				it++;
			}
		}

	}
	if ( valueSum == 0 )
		return;
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
				return;

			if ( data.size() == 33 || data.size() == 65 )
			{
				//make  hash
				CBitcoinAddress  address;
				CKeyID publicKey = CPubKey(data ).GetID();
				address.Set( publicKey );

				std::string key = address.ToString();

				CScript script;
				script = CScript() << CPubKey(data ) << OP_CHECKSIG;
				// optional solution script = CScript()  << OP_DUP << OP_HASH160 << *it << OP_EQUALVERIFY << OP_CHECKSIG;

				CTransaction txNew;
				txNew.vin.resize(1);
				txNew.vin[0].prevout.SetNull();
				txNew.vout.resize(1);
				txNew.vout[0].scriptPubKey = script;
				// this is  buggy right now
				txNew.vout[0].nValue = valueSum;
				//add transaction  to  pool and   view

				CTransactionRecordManager::getInstance()->addCoinbaseTransaction( txNew );
				return;
			}
		}


	}
}

}
