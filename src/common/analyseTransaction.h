// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ANALYSE_TRANSACTION_H
#define ANALYSE_TRANSACTION_H

class CTransaction;
class CTxOut;
class CKeyID;

namespace common
{

bool findOutputInTransaction( CTransaction const & _tx, CKeyID const & _findId , CTxOut & _txout, unsigned int & _id );

}

#endif // ANALYSE_TRANSACTION_H
