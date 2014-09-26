// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "core.h"
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
					if ( _findId == Hash160( *it ) )
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


}
