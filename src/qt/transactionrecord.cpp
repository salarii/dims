// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "transactionrecord.h"

#include "base58.h"
#include "wallet.h"

#include <stdint.h>

/* Return positive answer if transaction should be shown in list.
 */
bool TransactionRecord::showTransaction( CAvailableCoin const &wtx)
{
	return true;
}
/***
input  self ->  to  self
output self to outputs
zero self
**/
QList<TransactionRecord> TransactionRecord::decomposeTransaction(
		CWallet const * _wallet
		, CAvailableCoin const &_availableCoin
		, CKeyID const & _keyId )
{
	QList<TransactionRecord> parts;
	int64_t nTime = GetTime();
	uint256 hash = _availableCoin.m_hash;

	std::vector< CKeyID > inputs;

	CMnemonicAddress mnemonicAddress;
	mnemonicAddress.Set(_keyId);

	TransactionRecord sub(hash, nTime);

	sub.credit = _availableCoin.m_coin.nValue;

	sub.idx = parts.size(); // sequence number

	sub.address = mnemonicAddress.ToString();

	if ( !_wallet->getInputs( _availableCoin.m_hash, inputs ) )
	{

		sub.type = TransactionRecord::Generated;
	}
	else
	{
		std::set<CKeyID> addresses;
		_wallet->GetKeys( addresses );

		BOOST_FOREACH( CKeyID const & keyId, inputs )
		{
			if ( addresses.find( keyId ) != addresses.end() )
			{
				sub.type = TransactionRecord::SendToSelf;
				parts.append(sub);
				return parts;
			}
		}

		sub.type = TransactionRecord::RecvFromOther;
	}

	parts.append(sub);
	return parts;
}

/*
 * Decompose CWallet transaction to model transaction records.
 */
QList<TransactionRecord> TransactionRecord::decomposeTransaction(const CWallet *wallet, CTransaction const &wtx)
{
	QList<TransactionRecord> parts;
	int64_t nTime = GetTime();
	uint256 hash = wtx.GetHash();

	if ( wtx.IsCoinBase())
	{
		//
		// Credit
		//
		BOOST_FOREACH(const CTxOut& txout, wtx.vout)
		{
			if(wallet->IsMine(txout))
			{
				TransactionRecord sub(hash, nTime);
				CTxDestination address;
				sub.idx = parts.size(); // sequence number
				sub.credit = txout.nValue;
				if (ExtractDestination(txout.scriptPubKey, address) && IsMine(*wallet, address))
				{
					// Received by Bitcoin Address
					sub.type = TransactionRecord::RecvWithAddress;
					sub.address = CMnemonicAddress(address).ToString();
				}
				else
				{
					// Received by IP connection (deprecated features), or a multisignature or other non-simple transaction
					sub.type = TransactionRecord::RecvFromOther;
					// sub.address = mapValue["from"];
				}
				if (wtx.IsCoinBase())
				{
					// Generated
					sub.type = TransactionRecord::Generated;
				}

				parts.append(sub);
			}
		}
	}
	else
	{
		bool fAllFromMe = true;
		BOOST_FOREACH(const CTxIn& txin, wtx.vin)
				fAllFromMe = fAllFromMe;

		bool fAllToMe = true;
		BOOST_FOREACH(const CTxOut& txout, wtx.vout)
				fAllToMe = fAllToMe && wallet->IsMine(txout);

		if (fAllFromMe && fAllToMe)
		{
			// Payment to self

			parts.append(TransactionRecord(hash, nTime, TransactionRecord::SendToSelf, "",
										   -( 0), 0));
		}
		else if (fAllFromMe)
		{
			//
			// Debit
			//
			int64_t nTxFee = wtx.GetValueOut();

			for (unsigned int nOut = 0; nOut < wtx.vout.size(); nOut++)
			{
				const CTxOut& txout = wtx.vout[nOut];
				TransactionRecord sub(hash, nTime);
				sub.idx = parts.size();

				if(wallet->IsMine(txout))
				{
					// Ignore parts sent to self, as this is usually the change
					// from a transaction sent back to our own address.
					continue;
				}

				CTxDestination address;
				if (ExtractDestination(txout.scriptPubKey, address))
				{
					// Sent to Bitcoin Address
					sub.type = TransactionRecord::SendToAddress;
					sub.address = CMnemonicAddress(address).ToString();
				}
				else
				{
					// Sent to IP, or other non-address transaction like OP_EVAL
					sub.type = TransactionRecord::SendToOther;
					//         sub.address = mapValue["to"];
				}

				sub.debit = txout.nValue;

				parts.append(sub);
			}
		}
		else
		{
			//
			// Mixed debit transaction, can't break down payees
			//
			//   parts.append(TransactionRecord(hash, nTime, TransactionRecord::Other, "", nNet, 0));
		}
	}

	return parts;
}

void TransactionRecord::updateStatus(const CWalletTx &wtx)
{
    // Determine transaction status

    // Find the block the tx is in
    CBlockIndex* pindex = NULL;

    // Sort order, unrecorded transactions sort to the top
    status.sortKey = strprintf("%010d-%01d-%010u-%03d",
        (pindex ? pindex->nHeight : std::numeric_limits<int>::max()),
        (wtx.IsCoinBase() ? 1 : 0),
        wtx.nTimeReceived,
        idx);

    status.cur_num_blocks = chainActive.Height();

    if (!IsFinalTx(wtx, chainActive.Height() + 1))
    {
        if (wtx.nLockTime < LOCKTIME_THRESHOLD)
        {
            status.status = TransactionStatus::OpenUntilBlock;
            status.open_for = wtx.nLockTime - chainActive.Height();
        }
        else
        {
            status.status = TransactionStatus::OpenUntilDate;
            status.open_for = wtx.nLockTime;
        }
    }
    else
    {

    }

    // For generated transactions, determine maturity
    if(type == TransactionRecord::Generated)
    {
        int64_t nCredit = wtx.GetCredit(true);
        if (nCredit == 0)
        {
            status.maturity = TransactionStatus::Immature;

        }
        else
        {
            status.maturity = TransactionStatus::Mature;
        }
    }
}

bool TransactionRecord::statusUpdateNeeded()
{
    return status.cur_num_blocks != chainActive.Height();
}

QString TransactionRecord::getTxID() const
{
    return formatSubTxId(hash, idx);
}

QString TransactionRecord::formatSubTxId(const uint256 &hash, int vout)
{
    return QString::fromStdString(hash.ToString() + strprintf("-%03d", vout));
}

