// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "tracker/originAddressScaner.h"
#include "tracker/transactionRecordManager.h"
#include "originTransactionsDatabase.h"

#include "chainparams.h"
#include "base58.h"
#include "core.h"

#include "hash.h"

#include <vector>

namespace tracker
{

COriginAddressScanner * COriginAddressScanner::ms_instance = NULL;
	
COriginAddressScanner*
COriginAddressScanner::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new COriginAddressScanner();
	};
	return ms_instance;
}

COriginAddressScanner::COriginAddressScanner()
	: m_currentTime( 0 )
	, m_totalBalance( 0 )
{
}

void
COriginAddressScanner::addTransaction( uint64_t const _timeStamp, CTransaction const&  _tx)
{
	boost::lock_guard<boost::mutex> lock(m_lock);

	uint256 hash = _tx.GetHash();
	if ( m_alreadyProcessed.find( hash ) != m_alreadyProcessed.end() )
		return;

	m_alreadyProcessed.insert( hash );

	m_transactionToProcess.insert( std::make_pair( _timeStamp, _tx) );

	if ( m_currentTime != 0 || m_currentTime != _timeStamp )
	{
		COriginTransactionDatabase::getInstance()->storeOriginTransactionsFlush(
					  m_currentTime
					, m_totalBalance
					, m_keys
					, m_balances
					);
	}
	m_currentTime = _timeStamp;

}

void
COriginAddressScanner::updateTransactionRecord( uint64_t const _timeStamp )
{
	boost::lock_guard<boost::mutex> lock(m_lock);
	std::vector< std::vector< unsigned char > > keys;
	std::vector< uint64_t > balances;

	COriginTransactionDatabase::getInstance()->getOriginTransactions( _timeStamp, keys, balances );

	int i = 0;
	BOOST_FOREACH( std::vector< unsigned char > const & key, keys )
	{
		CKeyID publicKey = CPubKey( key ).GetID();

		CScript script;
		script = CScript() << CPubKey( key ) << OP_CHECKSIG;

		CTransaction txNew;
		txNew.vin.resize(1);
		txNew.vin[0].prevout.SetNull();
		txNew.vout.resize(1);
		txNew.vout[0].scriptPubKey = script;
		// this is  buggy right now
		txNew.vout[0].nValue = balances.at( i );
		//add transaction  to  pool and   view

		CTransactionRecordManager::getInstance()->addCoinbaseTransaction( txNew, publicKey );
		CTransactionRecordManager::getInstance()->addTransactionToStorage( txNew );
	}
}

void
COriginAddressScanner::loop()
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
		MilliSleep(50);
	}

}

void COriginAddressScanner::createBaseTransaction(CTransaction const &  _tx)
{
	unsigned int valueSum = 0;
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
				return;
		}
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
				m_balances.push_back( valueSum );
				m_keys.push_back( data );
				m_totalBalance += valueSum;

				CKeyID publicKey = CPubKey( data ).GetID();

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
				CTransactionRecordManager::getInstance()->addTransactionToStorage( txNew );
				CTransactionRecordManager::getInstance()->addCoinbaseTransaction( txNew, publicKey);
				return;

			}
		}
	}
}

}
