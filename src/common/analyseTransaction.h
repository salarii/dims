// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ANALYSE_TRANSACTION_H
#define ANALYSE_TRANSACTION_H

class CTransaction;
class CTxOut;
class CKeyID;

struct CAvailableCoin;

namespace common
{

bool findOutputInTransaction( CTransaction const & _tx, CKeyID const & _findId , CTxOut & _txout, unsigned int & _id );

std::vector< CAvailableCoin > getAvailableCoins( CCoins const & _coins, uint160 const & _pubId, uint256 const & _hash );

//signature looks crazy, but I need it this way, goal of this  function is to determine how many coins realy left input, needed for fee calculation
bool getRealCountOfCoinsSpend( CTransaction const & _tx, CKeyID const & _excaption, unsigned int & _outCount );// put tracker keyId in exceptions
}

#endif // ANALYSE_TRANSACTION_H
