#include "tracker/originAddressScaner.h"

#include "chainparams.h"

#include "core.h"

#include "hash.h"


#include <vector>

namespace Self
{


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

			}
		}
		boost::this_thread::interruption_point();
	}

}

void COriginAddressScaner::createBaseTransaction(CTransaction const &  _tx)
{
	for (unsigned int i = 0; i < _tx.vout.size(); i++)
	{
		const CTxOut& txout = _tx.vout[i];

		CScript::const_iterator pc = txout.scriptPubKey.begin();
		std::vector<unsigned char> data;

			opcodetype opcode;
		if (!txout.scriptPubKey.GetOp(pc, opcode, data))
			break;

		txnouttype type;
		unsigned int valueSum = 0;
		std::vector< std:: vector<unsigned char> > vSolutions;
		if (Solver(txout.scriptPubKey, type, vSolutions) &&
			(type == TX_PUBKEY || type == TX_PUBKEYHASH))
		{
			std::vector<std::vector<unsigned char> >::iterator it = vSolutions.begin();

			while( it != vSolutions.end() )
			{
				if ( type == TX_PUBKEY )
				{
					if ( uint160(ParseHex(Params().getOriginAddressAsString())) == Hash160( *it ) )
						valueSum += txout.nValue;
				}
				else
				{
					if ( uint160(ParseHex(Params().getOriginAddressAsString())) == uint160( ( *it ) ) )
						valueSum += txout.nValue;
				}
				it++;
			}
		}

		}
}
/*
if (fFound)
return true;

BOOST_FOREACH(const CTxIn& txin, _tx.vin)
{
	// Match if the filter contains any arbitrary script data element in any scriptSig in _tx
	CScript::const_iterator pc = txin.scriptSig.begin();
	vector<unsigned char> data;
	while (pc < txin.scriptSig.end())
	{
		opcodetype opcode;
		if (!txin.scriptSig.GetOp(pc, opcode, data))
			break;
		if (data.size() != 0 && contains(data))
			return true;
	}
//address  wysy³acza


	create  coin base transaction

		CPubKey pubkey;
	if (!reservekey.GetReservedKey(pubkey))
		return NULL;
OP_DUP << OP_HASH160 << OP_PUBKEYHASH << OP_EQUALVERIFY << OP_CHECKSIG
	CScript scriptPubKey = CScript() << pubkey << OP_CHECKSIG;

		// Create coinbase _tx
	CTransaction txNew;
	txNew.vin.resize(1);
	txNew.vin[0].prevout.SetNull();
	txNew.vout.resize(1);
	txNew.vout[0].scriptPubKey = scriptPubKeyIn;

}

return false;

}*/
}
