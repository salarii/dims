// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "wallet.h"

#include "base58.h"
#include "coincontrol.h"
#include "net.h"

#include <inttypes.h>

#include <boost/algorithm/string/replace.hpp>
#include <openssl/rand.h>

using namespace std;

// Settings
int64_t nTransactionFee = 0;
bool bSpendZeroConfChange = true;

//////////////////////////////////////////////////////////////////////////////
//
// mapWallet
//

struct CompareValueOnly2
{
	bool operator()(CAvailableCoin const & t1,
					CAvailableCoin const & t2) const
    {
		return t1.m_coin.nValue < t2.m_coin.nValue;
    }
};
struct CompareValueOnly
{
	bool operator()(const pair<int64_t, pair<const CWalletTx*, unsigned int> >& t1,
					const pair<int64_t, pair<const CWalletTx*, unsigned int> >& t2) const
	{
		return t1.first < t2.first;
	}
};

CPubKey CWallet::GenerateNewKey()
{
    AssertLockHeld(cs_wallet); // mapKeyMetadata
    bool fCompressed = CanSupportFeature(FEATURE_COMPRPUBKEY); // default to compressed public keys if we want 0.6.0 wallets

    RandAddSeedPerfmon();
    CKey secret;
    secret.MakeNewKey(fCompressed);

    // Compressed public keys were introduced in version 0.6.0
    if (fCompressed)
        SetMinVersion(FEATURE_COMPRPUBKEY);

    CPubKey pubkey = secret.GetPubKey();

    // Create new metadata
    int64_t nCreationTime = GetTime();
    mapKeyMetadata[pubkey.GetID()] = CKeyMetadata(nCreationTime);
    if (!nTimeFirstKey || nCreationTime < nTimeFirstKey)
        nTimeFirstKey = nCreationTime;

    if (!AddKeyPubKey(secret, pubkey))
        throw std::runtime_error("CWallet::GenerateNewKey() : AddKey failed");
    return pubkey;
}

bool CWallet::AddKeyPubKey(const CKey& secret, const CPubKey &pubkey, bool invisible)
{
    AssertLockHeld(cs_wallet); // mapKeyMetadata
    if (!CCryptoKeyStore::AddKeyPubKey(secret, pubkey))
        return false;
    if (!fFileBacked)
        return true;
    if (!IsCrypted())
    {
        if ( !CWalletDB(strWalletFile).WriteKey(pubkey,
                                                 secret.GetPrivKey(),
                                                 mapKeyMetadata[pubkey.GetID()]) )
	  {
								 return false;
	  }
    }

	if ( !invisible )
	{
		if (!SetAddressBook(pubkey.GetID(), "", "receive"))
			return false;
	}
	return true;
}

bool CWallet::RemoveKeyPubKey( const CPubKey &pubkey )
{
	AssertLockHeld(cs_wallet); // mapKeyMetadata
	if (!CCryptoKeyStore::RemoveKeyPubKey(pubkey))
		return false;
	if (!fFileBacked)
		return true;
	if (!IsCrypted())
	{
		if ( !CWalletDB(strWalletFile).EraseKey(pubkey) )
	  {
								 return false;
	  }
	}
	return true;
}

bool CWallet::AddCryptedKey(const CPubKey &vchPubKey,
                            const vector<unsigned char> &vchCryptedSecret)
{
    if (!CCryptoKeyStore::AddCryptedKey(vchPubKey, vchCryptedSecret))
        return false;
    if (!fFileBacked)
        return true;
    {
        LOCK(cs_wallet);
        if (pwalletdbEncryption)
            return pwalletdbEncryption->WriteCryptedKey(vchPubKey,
                                                        vchCryptedSecret,
                                                        mapKeyMetadata[vchPubKey.GetID()]);
        else
            return CWalletDB(strWalletFile).WriteCryptedKey(vchPubKey,
                                                            vchCryptedSecret,
                                                            mapKeyMetadata[vchPubKey.GetID()]);
    }
    return false;
}

bool CWallet::LoadKeyMetadata(const CPubKey &pubkey, const CKeyMetadata &meta)
{
    AssertLockHeld(cs_wallet); // mapKeyMetadata
    if (meta.nCreateTime && (!nTimeFirstKey || meta.nCreateTime < nTimeFirstKey))
        nTimeFirstKey = meta.nCreateTime;

    mapKeyMetadata[pubkey.GetID()] = meta;
    return true;
}

bool CWallet::LoadCryptedKey(const CPubKey &vchPubKey, const std::vector<unsigned char> &vchCryptedSecret)
{
    return CCryptoKeyStore::AddCryptedKey(vchPubKey, vchCryptedSecret);
}

bool CWallet::AddCScript(const CScript& redeemScript)
{
    if (!CCryptoKeyStore::AddCScript(redeemScript))
        return false;
    if (!fFileBacked)
        return true;
    return CWalletDB(strWalletFile).WriteCScript(Hash160(redeemScript), redeemScript);
}

bool CWallet::Unlock(const SecureString& strWalletPassphrase)
{
    CCrypter crypter;
    CKeyingMaterial vMasterKey;

    {
        LOCK(cs_wallet);
        BOOST_FOREACH(const MasterKeyMap::value_type& pMasterKey, mapMasterKeys)
        {
            if(!crypter.SetKeyFromPassphrase(strWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
                return false;
            if (!crypter.Decrypt(pMasterKey.second.vchCryptedKey, vMasterKey))
                continue; // try another master key
            if (CCryptoKeyStore::Unlock(vMasterKey))
                return true;
        }
    }
    return false;
}

bool CWallet::ChangeWalletPassphrase(const SecureString& strOldWalletPassphrase, const SecureString& strNewWalletPassphrase)
{
    bool fWasLocked = IsLocked();

    {
        LOCK(cs_wallet);
        Lock();

        CCrypter crypter;
        CKeyingMaterial vMasterKey;
        BOOST_FOREACH(MasterKeyMap::value_type& pMasterKey, mapMasterKeys)
        {
            if(!crypter.SetKeyFromPassphrase(strOldWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
                return false;
            if (!crypter.Decrypt(pMasterKey.second.vchCryptedKey, vMasterKey))
                return false;
            if (CCryptoKeyStore::Unlock(vMasterKey))
            {
                int64_t nStartTime = GetTimeMillis();
                crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod);
                pMasterKey.second.nDeriveIterations = pMasterKey.second.nDeriveIterations * (100 / ((double)(GetTimeMillis() - nStartTime)));

                nStartTime = GetTimeMillis();
                crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod);
                pMasterKey.second.nDeriveIterations = (pMasterKey.second.nDeriveIterations + pMasterKey.second.nDeriveIterations * 100 / ((double)(GetTimeMillis() - nStartTime))) / 2;

                if (pMasterKey.second.nDeriveIterations < 25000)
                    pMasterKey.second.nDeriveIterations = 25000;

                LogPrintf("Wallet passphrase changed to an nDeriveIterations of %i\n", pMasterKey.second.nDeriveIterations);

                if (!crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
                    return false;
                if (!crypter.Encrypt(vMasterKey, pMasterKey.second.vchCryptedKey))
                    return false;
                CWalletDB(strWalletFile).WriteMasterKey(pMasterKey.first, pMasterKey.second);
                if (fWasLocked)
                    Lock();
                return true;
            }
        }
    }

    return false;
}

bool CWallet::SetMinVersion(enum WalletFeature nVersion, CWalletDB* pwalletdbIn, bool fExplicit)
{
    AssertLockHeld(cs_wallet); // nWalletVersion
    if (nWalletVersion >= nVersion)
        return true;

    // when doing an explicit upgrade, if we pass the max version permitted, upgrade all the way
    if (fExplicit && nVersion > nWalletMaxVersion)
            nVersion = FEATURE_LATEST;

    nWalletVersion = nVersion;

    if (nVersion > nWalletMaxVersion)
        nWalletMaxVersion = nVersion;

    if (fFileBacked)
    {
        CWalletDB* pwalletdb = pwalletdbIn ? pwalletdbIn : new CWalletDB(strWalletFile);
        if (nWalletVersion > 40000)
            pwalletdb->WriteMinVersion(nWalletVersion);
        if (!pwalletdbIn)
            delete pwalletdb;
    }

    return true;
}

bool CWallet::SetMaxVersion(int nVersion)
{
    AssertLockHeld(cs_wallet); // nWalletVersion, nWalletMaxVersion
    // cannot downgrade below current version
    if (nWalletVersion > nVersion)
        return false;

    nWalletMaxVersion = nVersion;

    return true;
}

bool CWallet::EncryptWallet(const SecureString& strWalletPassphrase)
{
    if (IsCrypted())
        return false;

    CKeyingMaterial vMasterKey;
    RandAddSeedPerfmon();

    vMasterKey.resize(WALLET_CRYPTO_KEY_SIZE);
    RAND_bytes(&vMasterKey[0], WALLET_CRYPTO_KEY_SIZE);

    CMasterKey kMasterKey;

    RandAddSeedPerfmon();
    kMasterKey.vchSalt.resize(WALLET_CRYPTO_SALT_SIZE);
    RAND_bytes(&kMasterKey.vchSalt[0], WALLET_CRYPTO_SALT_SIZE);

    CCrypter crypter;
    int64_t nStartTime = GetTimeMillis();
    crypter.SetKeyFromPassphrase(strWalletPassphrase, kMasterKey.vchSalt, 25000, kMasterKey.nDerivationMethod);
    kMasterKey.nDeriveIterations = 2500000 / ((double)(GetTimeMillis() - nStartTime));

    nStartTime = GetTimeMillis();
    crypter.SetKeyFromPassphrase(strWalletPassphrase, kMasterKey.vchSalt, kMasterKey.nDeriveIterations, kMasterKey.nDerivationMethod);
    kMasterKey.nDeriveIterations = (kMasterKey.nDeriveIterations + kMasterKey.nDeriveIterations * 100 / ((double)(GetTimeMillis() - nStartTime))) / 2;

    if (kMasterKey.nDeriveIterations < 25000)
        kMasterKey.nDeriveIterations = 25000;

    LogPrintf("Encrypting Wallet with an nDeriveIterations of %i\n", kMasterKey.nDeriveIterations);

    if (!crypter.SetKeyFromPassphrase(strWalletPassphrase, kMasterKey.vchSalt, kMasterKey.nDeriveIterations, kMasterKey.nDerivationMethod))
        return false;
    if (!crypter.Encrypt(vMasterKey, kMasterKey.vchCryptedKey))
        return false;

    {
        LOCK(cs_wallet);
        mapMasterKeys[++nMasterKeyMaxID] = kMasterKey;
        if (fFileBacked)
        {
            pwalletdbEncryption = new CWalletDB(strWalletFile);
            if (!pwalletdbEncryption->TxnBegin())
                return false;
            pwalletdbEncryption->WriteMasterKey(nMasterKeyMaxID, kMasterKey);
        }

        if (!EncryptKeys(vMasterKey))
        {
            if (fFileBacked)
                pwalletdbEncryption->TxnAbort();
            exit(1); //We now probably have half of our keys encrypted in memory, and half not...die and let the user reload their unencrypted wallet.
        }

        // Encryption was introduced in version 0.4.0
        SetMinVersion(FEATURE_WALLETCRYPT, pwalletdbEncryption, true);

        if (fFileBacked)
        {
            if (!pwalletdbEncryption->TxnCommit())
                exit(1); //We now have keys encrypted in memory, but no on disk...die to avoid confusion and let the user reload their unencrypted wallet.

            delete pwalletdbEncryption;
            pwalletdbEncryption = NULL;
        }

        Lock();
        Unlock(strWalletPassphrase);
        NewKeyPool();
        Lock();

        // Need to completely rewrite the wallet file; if we don't, bdb might keep
        // bits of the unencrypted private key in slack space in the database file.
        CDB::Rewrite(strWalletFile);

    }
    NotifyStatusChanged(this);

    return true;
}

int64_t CWallet::IncOrderPosNext(CWalletDB *pwalletdb)
{
    AssertLockHeld(cs_wallet); // nOrderPosNext
    int64_t nRet = nOrderPosNext++;
    if (pwalletdb) {
        pwalletdb->WriteOrderPosNext(nOrderPosNext);
    } else {
        CWalletDB(strWalletFile).WriteOrderPosNext(nOrderPosNext);
    }
    return nRet;
}

bool CWallet::IsChange(const CTxOut& txout) const
{
    CTxDestination address;

    // TODO: fix handling of 'change' outputs. The assumption is that any
    // payment to a TX_PUBKEYHASH that is mine but isn't in the address book
    // is change. That assumption is likely to break when we implement multisignature
    // wallets that return change back into a multi-signature-protected address;
    // a better way of identifying which outputs are 'the send' and which are
    // 'the change' will need to be implemented (maybe extend CWalletTx to remember
    // which output, if any, was change).
    if (ExtractDestination(txout.scriptPubKey, address) && ::IsMine(*this, address))
    {
        LOCK(cs_wallet);
        if (!mapAddressBook.count(address))
            return true;
    }
    return false;
}

int64_t CWalletTx::GetTxTime() const
{
    int64_t n = nTimeSmart;
    return n ? n : nTimeReceived;
}

int CWalletTx::GetRequestCount() const
{
    // Returns -1 if it wasn't being tracked
    int nRequests = -1;
    {
        LOCK(pwallet->cs_wallet);

        {
            // Did anyone request this transaction?
            map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(GetHash());
            if (mi != pwallet->mapRequestCount.end())
            {
                nRequests = (*mi).second;


            }
        }
    }
    return nRequests;
}


bool CWalletTx::WriteToDisk()
{
    return CWalletDB(pwallet->strWalletFile).WriteTx(GetHash(), *this);
}

// Scan the block chain (starting in pindexStart) for transactions
// from or to us. If fUpdate is true, found transactions that already
// exist in the wallet will be updated.
int CWallet::ScanForWalletTransactions(CBlockIndex* pindexStart, bool fUpdate)
{
    int ret = 0;

    CBlockIndex* pindex = pindexStart;
    {
        LOCK(cs_wallet);
        while (pindex)
        {
            // no need to read and scan block, if block was created before
            // our wallet birthday (as adjusted for block time variability)
            if (nTimeFirstKey && (pindex->nTime < (nTimeFirstKey - 7200))) {
                pindex = chainActive.Next(pindex);
                continue;
            }

            pindex = chainActive.Next(pindex);
        }
    }
    return ret;
}

set<uint256> CWalletTx::GetConflicts() const
{
    set<uint256> result;
    if (pwallet != NULL)
    {
        uint256 myHash = GetHash();
        result.erase(myHash);
    }
    return result;
}



CWallet * CWallet::ms_instance = NULL;

CWallet*
CWallet::getInstance( std::string const & _fileName )
{
	if ( !ms_instance )
	{
		ms_instance = new CWallet( _fileName );
	};
	return ms_instance;
}

CWallet*
CWallet::getInstance( )
{
	return ms_instance;
}
//////////////////////////////////////////////////////////////////////////////
//
// Actions
//


int64_t CWallet::GetBalance() const
{
	std::multimap< uint160, CAvailableCoin >::const_iterator iterator = m_availableCoins.begin();

	uint64_t balance = 0;

	while( iterator != m_availableCoins.end() )
	{
		balance += iterator->second.m_coin.nValue;
		iterator++;
	}
	return balance;
	/*
    int64_t nTotal = 0;
    {
        LOCK(cs_wallet);
        for (map<uint256, CWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const CWalletTx* pcoin = &(*it).second;
            if (pcoin->IsTrusted())
                nTotal += pcoin->GetAvailableCredit();
        }
    }

	return nTotal;*/
}

int64_t CWallet::GetUnconfirmedBalance() const
{
    int64_t nTotal = 0;

    return nTotal;
}

bool CWallet::getKeyForCoin(CAvailableCoin const & _availableCoin, CKeyID & _keyId ) const
{
	BOOST_FOREACH( PAIRTYPE( uint160, CAvailableCoin ) const & coin, m_availableCoins )
	{
		if ( coin.second == _availableCoin )
		{
			_keyId = coin.first;
			return true;
		}
	}
	return false;
}

uint64_t CWallet::AvailableCoinsAmount(CKeyID const & _keyID) const
{
	std::multimap< uint160, CAvailableCoin >::const_iterator low = m_availableCoins.lower_bound(_keyID), up = m_availableCoins.upper_bound(_keyID);

	uint64_t balance = 0;

	while( low != up )
	{
		balance += low->second.m_coin.nValue;
		low++;
	}
	return balance;
}

unsigned CWallet::AllAvailableCoinsAmount() const
{
	return 0;
}

static void ApproximateBestSubset(vector<pair<int64_t, pair<const CWalletTx*,unsigned int> > >vValue, int64_t nTotalLower, int64_t nTargetValue,
                                  vector<char>& vfBest, int64_t& nBest, int iterations = 1000)
{
    vector<char> vfIncluded;

    vfBest.assign(vValue.size(), true);
    nBest = nTotalLower;

    seed_insecure_rand();

    for (int nRep = 0; nRep < iterations && nBest != nTargetValue; nRep++)
    {
        vfIncluded.assign(vValue.size(), false);
        int64_t nTotal = 0;
        bool fReachedTarget = false;
        for (int nPass = 0; nPass < 2 && !fReachedTarget; nPass++)
        {
            for (unsigned int i = 0; i < vValue.size(); i++)
            {
                //The solver here uses a randomized algorithm,
                //the randomness serves no real security purpose but is just
                //needed to prevent degenerate behavior and it is important
                //that the rng fast. We do not use a constant random sequence,
                //because there may be some privacy improvement by making
                //the selection random.
                if (nPass == 0 ? insecure_rand()&1 : !vfIncluded[i])
                {
                    nTotal += vValue[i].first;
                    vfIncluded[i] = true;
                    if (nTotal >= nTargetValue)
                    {
                        fReachedTarget = true;
                        if (nTotal < nBest)
                        {
                            nBest = nTotal;
                            vfBest = vfIncluded;
                        }
                        nTotal -= vValue[i].first;
                        vfIncluded[i] = false;
                    }
                }
            }
        }
    }
}

bool CWallet::SelectCoins(int64_t nTargetValue, std::vector<CAvailableCoin> & setCoinsRet, int64_t& nValueRet, const CCoinControl* coinControl) const
{
		setCoinsRet.clear();
		nValueRet = 0;

		// List of values less than target
		pair<int64_t,CAvailableCoin const*> coinLowestLarger;
		coinLowestLarger.first = std::numeric_limits<int64_t>::max();
		coinLowestLarger.second = NULL;
		std::vector< CAvailableCoin > vValue;
		int64_t nTotalLower = 0;
//reference here is important
		CAvailableCoin coin;
		BOOST_FOREACH( PAIRTYPE( uint160, CAvailableCoin ) const & coinPair, m_availableCoins)
		{
			int64_t n = coinPair.second.m_coin.nValue;

			if (n == nTargetValue)
			{
				setCoinsRet.push_back(coinPair.second);
				nValueRet += coinPair.second.m_coin.nValue;
				return true;
			}
			else if (n < nTargetValue )
			{
				vValue.push_back(coinPair.second);
				nTotalLower += n;
			}
			else if (n < coinLowestLarger.first)
			{
				coin = coinPair.second;
				coinLowestLarger = std::make_pair(n, &coin);
			}
		}

		if (nTotalLower == nTargetValue)
		{
			setCoinsRet = vValue;

			nValueRet = nTargetValue;
			return true;
		}

		if (nTotalLower < nTargetValue)
		{
			if (coinLowestLarger.second == NULL)
				return false;
			setCoinsRet.push_back(*coinLowestLarger.second);
			nValueRet += coinLowestLarger.first;
			return true;
		}

		// Solve subset sum by stochastic approximation
		sort(vValue.rbegin(), vValue.rend(), CompareValueOnly2());
		//vector<char> vfBest;
		//int64_t nBest;

		//disable fency stuff for now in order  to make  debuging/development easier/faster
		/*
		ApproximateBestSubset(vValue, nTotalLower, nTargetValue, vfBest, nBest, 1000);
		if (nBest != nTargetValue && nTotalLower >= nTargetValue + CENT)
			ApproximateBestSubset(vValue, nTotalLower, nTargetValue + CENT, vfBest, nBest, 1000);
*/
		// If we have a bigger coin and (either the stochastic approximation didn't find a good solution,
		//                                   or the next bigger coin is closer), return the bigger coin
		/*if (coinLowestLarger.second.first &&
			((nBest != nTargetValue && nBest < nTargetValue + CENT) || coinLowestLarger.first <= nBest))
		{
			setCoinsRet.insert(coinLowestLarger.second);
			nValueRet += coinLowestLarger.first;
		}
		else {*/
		std::vector< CAvailableCoin >::iterator iterator = vValue.begin();
		while( iterator != vValue.end() )
		{
			if (nTargetValue >= nValueRet )
			{
				setCoinsRet.push_back(*iterator);
				nValueRet += iterator->m_coin.nValue;
			}
			else
			{
				return true;
			}
			iterator++;
		}

		if (nTargetValue <= nValueRet )
			return true;
/*
			LogPrint("selectcoins", "SelectCoins() best subset: ");
			for (unsigned int i = 0; i < vValue.size(); i++)
				if (vfBest[i])
					LogPrint("selectcoins", "%s ", FormatMoney(vValue[i].first));
			LogPrint("selectcoins", "total %s\n", FormatMoney(nBest));

			*/

		return false;
}

bool
CWallet::CreateTransaction( std::vector< std::pair< CKeyID, int64_t > > const & _outputs, std::vector< CSpendCoins > const & _coinsToUse, CPubKey const & _trackerKey, unsigned int _price, CWalletTx& wtxNew, std::string& strFailReason )
{
	CUpdateCoins updateCoins;
	std::map< int, CKeyID > mine;
	std::vector<std::pair<CScript, int64_t> > vecSend;

	int id = 0;
	int64_t total = 0;
	BOOST_FOREACH( PAIRTYPE( CKeyID, int64_t ) const & _target, _outputs )
	{
		if (_target.second < 0)
		{
			strFailReason = _("Transaction amounts must be positive");
			return false;
		}
		total += _target.second;
		CScript scriptPubKey;
		scriptPubKey.SetDestination( CTxDestination( _target.first ) );
		vecSend.push_back( std::pair<CScript, int64_t>( scriptPubKey, _target.second ) );
		if ( IsMine( _target.first ) )
			mine.insert( std::make_pair( id, _target.first ) );
		id++;
	}

	if (vecSend.empty())
	{
		strFailReason = _("Transaction amounts must be positive");
		return false;
	}

	int64_t trackerFee = _price;

	{
		CScript scriptPubKey;
		scriptPubKey.SetDestination( CTxDestination( _trackerKey.GetID() ) );
		vecSend.push_back( std::pair<CScript, int64_t>( scriptPubKey, trackerFee ) );
	}

	int64_t nValue = total + trackerFee;
	{
		LOCK2(cs_main, cs_wallet);
		{
			while (true)
			{
				wtxNew.vin.clear();
				wtxNew.vout.clear();
				wtxNew.fFromMe = true;

				int64_t nTotalValue = nValue;
				// vouts to the payees
				id = 0;
				BOOST_FOREACH (const PAIRTYPE(CScript, int64_t)& s, vecSend)
				{
					CTxOut txout(s.second, s.first);
					wtxNew.vout.push_back(txout);

					std::map< int, CKeyID >::iterator mineIterator = mine.find( id );
					if ( mineIterator != mine.end() )
						updateCoins.m_toAdd.insert( std::make_pair( mineIterator->second, CAvailableCoin( txout, -1, -1 ) ) );
					id++;
				}

				int64_t requestedValueIn = 0;
				std::vector< CSpendCoins > requestedSetCoins;

				std::vector< CSpendCoins >::const_iterator iterator = _coinsToUse.begin();

				while( iterator != _coinsToUse.end() )
				{
					nTotalValue -= iterator->m_coin.nValue;
					requestedValueIn += iterator->m_coin.nValue;
					requestedSetCoins.push_back( *iterator );
					if ( nTotalValue <= 0 )
					{
						nTotalValue = 0;
						break;
					}
					iterator++;
				}

				int64_t nValueIn = 0;
				std::vector< CAvailableCoin > setCoins;

				if (!SelectCoins(nTotalValue, setCoins, nValueIn, 0))
				{
					strFailReason = _("Insufficient funds");
					return false;
				}

				nValueIn += requestedValueIn;

				//setCoins.insert( setCoins.end(), requestedSetCoins.begin(), requestedSetCoins.end() );

				int64_t nChange = nValueIn - nValue;
//this  I will fix later
				if (nChange > 0)
				{
					// Fill a vout to ourself
					// TODO: pass in scriptChange instead of reservekey so
					// change transaction isn't always pay-to-bitcoin-address
					CScript scriptChange;

					CKeyID keyId;
					if ( !determineChangeAddress( setCoins, keyId ) )
						return false;
					scriptChange.SetDestination(keyId);

					CTxOut newTxOut(nChange, scriptChange);

				  // Insert change txn at random position:
				  vector<CTxOut>::iterator position = wtxNew.vout.begin()+GetRandInt(wtxNew.vout.size()+1);
				  wtxNew.vout.insert(position, newTxOut);

				  updateCoins.m_toAdd.insert( std::make_pair( keyId, CAvailableCoin( newTxOut, -1, -1 ) ) );

				}

				BOOST_FOREACH( CAvailableCoin const & coin, setCoins)
				{
					CKeyID keyId;
					if ( !getKeyForCoin( coin, keyId ) )
						assert( !"coin should be present here" );
					updateCoins.m_toRemove.insert( std::make_pair( keyId, coin ) );

				}

				// Fill vin
				BOOST_FOREACH( CAvailableCoin const & coin, setCoins)
					wtxNew.vin.push_back(CTxIn(coin.m_hash,coin.m_position));

				CBasicKeyStore basicKeyStore;

				BOOST_FOREACH( CSpendCoins const & spendCoin, requestedSetCoins )
				{
					basicKeyStore.AddKeyPubKey( spendCoin.m_key, spendCoin.m_key.GetPubKey() );
				}

				BOOST_FOREACH( CSpendCoins const & coin, requestedSetCoins )
					wtxNew.vin.push_back(CTxIn(coin.m_hash,coin.m_position));

				// Sign
				int nIn = 0;
				BOOST_FOREACH( CAvailableCoin const & coin, setCoins)
				if (!SignSignature(*this, coin.m_coin.scriptPubKey, wtxNew, nIn++) )
				{
						strFailReason = _("Signing transaction failed");
						return false;
				}

				BOOST_FOREACH( CSpendCoins const & coin, requestedSetCoins)
				if (!SignSignature(basicKeyStore, coin.m_coin.scriptPubKey, wtxNew, nIn++) )
				{
						strFailReason = _("Signing transaction failed");
						return false;
				}

				// Limit size
				unsigned int nBytes = ::GetSerializeSize(*(CTransaction*)&wtxNew, SER_NETWORK, PROTOCOL_VERSION);
				if (nBytes >= MAX_STANDARD_TX_SIZE)
				{
					strFailReason = _("Transaction too large");
					return false;
				}

			/*
			// don't know  what for is logic below

				wtxNew.AddSupportingTransactions();
				wtxNew.fTimeReceivedIsTxTime = true;
			*/
				break;
			}
		}
	}

	m_waitingChanges.insert( std::make_pair( wtxNew.GetHash(), updateCoins ) );
	return true;
}

void
CWallet::addmitNewTransaction( uint256 const & _initialHash, CTransaction const & _addmitedTransaction )
{
	AssertLockHeld(cs_wallet);

	std::map< uint256, CUpdateCoins >::iterator iterator = m_waitingChanges.find( _initialHash );

	if ( iterator == m_waitingChanges.end() )
	{
		assert( !"can't happen" );
		return;
	}
	uint256 hash = _addmitedTransaction.GetHash();

	std::vector< CAvailableCoin > remove;
	std::vector< CAvailableCoin > add;

	CKeyID keyId = CKeyID( 0 );

	BOOST_FOREACH( PAIRTYPE( CKeyID, CAvailableCoin ) const & coins, iterator->second.m_toRemove )
	{
		if ( keyId != coins.first )
		{
			if ( !remove.empty() )
			{
				removeCoins( keyId, remove );
				NotifyAddressBookChanged(this, keyId, "", true, "", CT_BALANCE);
			}
			remove.clear();
			keyId = coins.first;
		}

		remove.push_back( coins.second );
	}

	if ( !remove.empty() )
	{
		removeCoins( keyId, remove );
		NotifyAddressBookChanged(this, keyId, "", true, "", CT_BALANCE);
	}


	keyId = CKeyID( 0 );
	BOOST_FOREACH( PAIRTYPE( CKeyID const, CAvailableCoin ) & coins, iterator->second.m_toAdd )
	{
		if ( keyId != coins.first )
		{
			if ( !add.empty() )
			{
				addAvailableCoins( keyId, add );
				NotifyAddressBookChanged(this, keyId, "", true, "", CT_BALANCE);
			}
			add.clear();
			keyId = coins.first;
		}

		std::vector<CTxOut>::const_iterator txOutIterator = std::find( _addmitedTransaction.vout.begin(), _addmitedTransaction.vout.end(), coins.second.m_coin );
		if ( txOutIterator == _addmitedTransaction.vout.end() )
		{
			assert( !"can't happen" );
			return;
		}
		size_t distance = txOutIterator - _addmitedTransaction.vout.begin();

		coins.second.m_position = distance;
		coins.second.m_hash = hash;

		add.push_back( coins.second );
	}

	if ( !add.empty() )
	{
		addAvailableCoins( keyId, add );
		NotifyAddressBookChanged(this, keyId, "", true, "", CT_BALANCE);
	}

}

bool CWallet::determineChangeAddress( std::vector< CAvailableCoin > const & _coinsForTransaction, CKeyID & _keyId )
{
	std::multimap< uint160, CAvailableCoin >::const_iterator iterator = m_availableCoins.begin();

	while( iterator != m_availableCoins.end() )
	{
		if ( std::find(_coinsForTransaction.begin(), _coinsForTransaction.end(), iterator->second ) != _coinsForTransaction.end() )
		{
			_keyId = iterator->first;
			return true;
		}
		iterator++;
	}
	return false;
}

DBErrors CWallet::LoadWallet(bool& fFirstRunRet)
{
    if (!fFileBacked)
        return DB_LOAD_OK;
    fFirstRunRet = false;
    DBErrors nLoadWalletRet = CWalletDB(strWalletFile,"cr+").LoadWallet(this);
    if (nLoadWalletRet == DB_NEED_REWRITE)
    {
        if (CDB::Rewrite(strWalletFile, "\x04pool"))
        {
            LOCK(cs_wallet);
            setKeyPool.clear();
            // Note: can't top-up keypool here, because wallet is locked.
            // User will be prompted to unlock wallet the next operation
            // the requires a new key.
        }
    }

    if (nLoadWalletRet != DB_LOAD_OK)
        return nLoadWalletRet;
    fFirstRunRet = !vchDefaultKey.IsValid();

    return DB_LOAD_OK;
}


DBErrors CWallet::ZapWalletTx()
{
    if (!fFileBacked)
        return DB_LOAD_OK;
    DBErrors nZapWalletTxRet = CWalletDB(strWalletFile,"cr+").ZapWalletTx(this);
    if (nZapWalletTxRet == DB_NEED_REWRITE)
    {
        if (CDB::Rewrite(strWalletFile, "\x04pool"))
        {
            LOCK(cs_wallet);
            setKeyPool.clear();
            // Note: can't top-up keypool here, because wallet is locked.
            // User will be prompted to unlock wallet the next operation
            // the requires a new key.
        }
    }

    if (nZapWalletTxRet != DB_LOAD_OK)
        return nZapWalletTxRet;

    return DB_LOAD_OK;
}


bool CWallet::SetAddressBook(const CTxDestination& address, const string& strName, const string& strPurpose)
{
    AssertLockHeld(cs_wallet); // mapAddressBook
    std::map<CTxDestination, CAddressBookData>::iterator mi = mapAddressBook.find(address);
    mapAddressBook[address].name = strName;
    if (!strPurpose.empty()) /* update purpose only if requested */
        mapAddressBook[address].purpose = strPurpose;
    NotifyAddressBookChanged(this, address, strName, ::IsMine(*this, address),
            mapAddressBook[address].purpose,
            (mi == mapAddressBook.end()) ?  CT_NEW : CT_UPDATED);
    if (!fFileBacked)
        return false;
	if (!strPurpose.empty() && !CWalletDB(strWalletFile).WritePurpose(CMnemonicAddress(address).ToString(), strPurpose))
        return false;
	return CWalletDB(strWalletFile).WriteName(CMnemonicAddress(address).ToString(), strName);
}

bool CWallet::DelAddressBook(const CTxDestination& address)
{

    AssertLockHeld(cs_wallet); // mapAddressBook

    if(fFileBacked)
    {
        // Delete destdata tuples associated with address
		std::string strAddress = CMnemonicAddress(address).ToString();
        BOOST_FOREACH(const PAIRTYPE(string, string) &item, mapAddressBook[address].destdata)
        {
            CWalletDB(strWalletFile).EraseDestData(strAddress, item.first);
        }
    }

    mapAddressBook.erase(address);
    NotifyAddressBookChanged(this, address, "", ::IsMine(*this, address), "", CT_DELETED);
    if (!fFileBacked)
        return false;
	CWalletDB(strWalletFile).ErasePurpose(CMnemonicAddress(address).ToString());
	return CWalletDB(strWalletFile).EraseName(CMnemonicAddress(address).ToString());
}

bool CWallet::SetDefaultKey(const CPubKey &vchPubKey)
{
    if (fFileBacked)
    {
        if (!CWalletDB(strWalletFile).WriteDefaultKey(vchPubKey))
            return false;
    }
    vchDefaultKey = vchPubKey;
    return true;
}

//
// Mark old keypool keys as used,
// and generate all new keys
//
bool CWallet::NewKeyPool()
{
    {
        LOCK(cs_wallet);
        CWalletDB walletdb(strWalletFile);
        BOOST_FOREACH(int64_t nIndex, setKeyPool)
            walletdb.ErasePool(nIndex);
        setKeyPool.clear();

        if (IsLocked())
            return false;

        int64_t nKeys = max(GetArg("-keypool", 100), (int64_t)0);
        for (int i = 0; i < nKeys; i++)
        {
            int64_t nIndex = i+1;
            walletdb.WritePool(nIndex, CKeyPool(GenerateNewKey()));
            setKeyPool.insert(nIndex);
        }
        LogPrintf("CWallet::NewKeyPool wrote %"PRId64" new keys\n", nKeys);
    }
    return true;
}

bool CWallet::TopUpKeyPool(unsigned int kpSize)
{
    {
        LOCK(cs_wallet);

        if (IsLocked())
            return false;

        CWalletDB walletdb(strWalletFile);

        // Top up key pool
        unsigned int nTargetSize;
        if (kpSize > 0)
            nTargetSize = kpSize;
        else
            nTargetSize = max(GetArg("-keypool", 100), (int64_t) 0);

        while (setKeyPool.size() < (nTargetSize + 1))
        {
            int64_t nEnd = 1;
            if (!setKeyPool.empty())
                nEnd = *(--setKeyPool.end()) + 1;
            if (!walletdb.WritePool(nEnd, CKeyPool(GenerateNewKey())))
                throw runtime_error("TopUpKeyPool() : writing generated key failed");
            setKeyPool.insert(nEnd);
            LogPrintf("keypool added key %"PRId64", size=%"PRIszu"\n", nEnd, setKeyPool.size());
        }
    }
    return true;
}

void CWallet::ReserveKeyFromKeyPool(int64_t& nIndex, CKeyPool& keypool)
{
    nIndex = -1;
    keypool.vchPubKey = CPubKey();
    {
        LOCK(cs_wallet);

        if (!IsLocked())
            TopUpKeyPool();

        // Get the oldest key
        if(setKeyPool.empty())
            return;

        CWalletDB walletdb(strWalletFile);

        nIndex = *(setKeyPool.begin());
        setKeyPool.erase(setKeyPool.begin());
        if (!walletdb.ReadPool(nIndex, keypool))
            throw runtime_error("ReserveKeyFromKeyPool() : read failed");
        if (!HaveKey(keypool.vchPubKey.GetID()))
            throw runtime_error("ReserveKeyFromKeyPool() : unknown key in key pool");
        assert(keypool.vchPubKey.IsValid());
        LogPrintf("keypool reserve %"PRId64"\n", nIndex);
    }
}

int64_t CWallet::AddReserveKey(const CKeyPool& keypool)
{
    {
        LOCK2(cs_main, cs_wallet);
        CWalletDB walletdb(strWalletFile);

        int64_t nIndex = 1 + *(--setKeyPool.end());
        if (!walletdb.WritePool(nIndex, keypool))
            throw runtime_error("AddReserveKey() : writing added key failed");
        setKeyPool.insert(nIndex);
        return nIndex;
    }
    return -1;
}

void CWallet::KeepKey(int64_t nIndex)
{
    // Remove from key pool
    if (fFileBacked)
    {
        CWalletDB walletdb(strWalletFile);
        walletdb.ErasePool(nIndex);
    }
    LogPrintf("keypool keep %"PRId64"\n", nIndex);
}

void CWallet::ReturnKey(int64_t nIndex)
{
    // Return to key pool
    {
        LOCK(cs_wallet);
        setKeyPool.insert(nIndex);
    }
    LogPrintf("keypool return %"PRId64"\n", nIndex);
}

bool CWallet::GetKeyFromPool(CPubKey& result)
{
    int64_t nIndex = 0;
    CKeyPool keypool;
    {
        LOCK(cs_wallet);
        ReserveKeyFromKeyPool(nIndex, keypool);
        if (nIndex == -1)
        {
            if (IsLocked()) return false;
            result = GenerateNewKey();
            return true;
        }
        KeepKey(nIndex);
        result = keypool.vchPubKey;
    }
    return true;
}

int64_t CWallet::GetOldestKeyPoolTime()
{
    int64_t nIndex = 0;
    CKeyPool keypool;
    ReserveKeyFromKeyPool(nIndex, keypool);
    if (nIndex == -1)
        return GetTime();
    ReturnKey(nIndex);
    return keypool.nTime;
}

set<CTxDestination> CWallet::GetAccountAddresses(string strAccount) const
{
    AssertLockHeld(cs_wallet); // mapWallet
    set<CTxDestination> result;
    BOOST_FOREACH(const PAIRTYPE(CTxDestination, CAddressBookData)& item, mapAddressBook)
    {
        const CTxDestination& address = item.first;
        const string& strName = item.second.name;
        if (strName == strAccount)
            result.insert(address);
    }
    return result;
}

bool CReserveKey::GetReservedKey(CPubKey& pubkey)
{
    if (nIndex == -1)
    {
        CKeyPool keypool;
        pwallet->ReserveKeyFromKeyPool(nIndex, keypool);
        if (nIndex != -1)
            vchPubKey = keypool.vchPubKey;
        else {
            if (pwallet->vchDefaultKey.IsValid()) {
                LogPrintf("CReserveKey::GetReservedKey(): Warning: Using default key instead of a new key, top up your keypool!");
                vchPubKey = pwallet->vchDefaultKey;
            } else
                return false;
        }
    }
    assert(vchPubKey.IsValid());
    pubkey = vchPubKey;
    return true;
}

void CReserveKey::KeepKey()
{
    if (nIndex != -1)
        pwallet->KeepKey(nIndex);
    nIndex = -1;
    vchPubKey = CPubKey();
}

void CReserveKey::ReturnKey()
{
    if (nIndex != -1)
        pwallet->ReturnKey(nIndex);
    nIndex = -1;
    vchPubKey = CPubKey();
}

void CWallet::GetAllReserveKeys(set<CKeyID>& setAddress) const
{
    setAddress.clear();

    CWalletDB walletdb(strWalletFile);

    LOCK2(cs_main, cs_wallet);
    BOOST_FOREACH(const int64_t& id, setKeyPool)
    {
        CKeyPool keypool;
        if (!walletdb.ReadPool(id, keypool))
            throw runtime_error("GetAllReserveKeyHashes() : read failed");
        assert(keypool.vchPubKey.IsValid());
        CKeyID keyID = keypool.vchPubKey.GetID();
        if (!HaveKey(keyID))
            throw runtime_error("GetAllReserveKeyHashes() : unknown key in key pool");
        setAddress.insert(keyID);
    }
}

void CWallet::LockCoin(COutPoint& output)
{
    AssertLockHeld(cs_wallet); // setLockedCoins
    setLockedCoins.insert(output);
}

void CWallet::UnlockCoin(COutPoint& output)
{
    AssertLockHeld(cs_wallet); // setLockedCoins
    setLockedCoins.erase(output);
}

void CWallet::UnlockAllCoins()
{
    AssertLockHeld(cs_wallet); // setLockedCoins
    setLockedCoins.clear();
}

bool CWallet::IsLockedCoin(uint256 hash, unsigned int n) const
{
    AssertLockHeld(cs_wallet); // setLockedCoins
    COutPoint outpt(hash, n);

    return (setLockedCoins.count(outpt) > 0);
}

void CWallet::ListLockedCoins(std::vector<COutPoint>& vOutpts)
{
    AssertLockHeld(cs_wallet); // setLockedCoins
    for (std::set<COutPoint>::iterator it = setLockedCoins.begin();
         it != setLockedCoins.end(); it++) {
        COutPoint outpt = (*it);
        vOutpts.push_back(outpt);
    }
}

void
CWallet::addInputs( std::map< uint256, std::vector< CKeyID > > const & _inputs )
{
	AssertLockHeld(cs_wallet);
	m_inputs.insert( _inputs.begin(), _inputs.end() );
}


void
CWallet::addAvailableCoins( CKeyID const & _keyId, std::vector< CAvailableCoin > const & _availableCoins, bool _writeToDatabase )
{
	AssertLockHeld(cs_wallet);

	addCoins( _keyId, _availableCoins );

	if ( _writeToDatabase )
	{
		if ( !CWalletDB(this->strWalletFile).addCoins( _keyId, _availableCoins ) )
			assert( !"fail to write coin" );
	}
	NotifyAddressBookChanged(this, _keyId, "", true, "", CT_BALANCE);
}

void
CWallet::replaceAvailableCoins( CKeyID const & _keyId, std::vector< CAvailableCoin > const & _availableCoins )
{
	AssertLockHeld(cs_wallet);

	m_availableCoins.erase(_keyId);

	if ( !CWalletDB(this->strWalletFile).replaceCoins( _keyId, _availableCoins ) )
		assert( !"fail to erase coin" );

	addCoins( _keyId, _availableCoins );

	NotifyAddressBookChanged(this, _keyId, "", true, "", CT_BALANCE);
}

void
CWallet::removeCoins( CKeyID const & _keyId, std::vector< CAvailableCoin > const & _previousCoins )
{
	std::vector< CAvailableCoin > left;

	for ( std::multimap< uint160, CAvailableCoin >::iterator iterator = m_availableCoins.lower_bound( _keyId ), previous = iterator; iterator != m_availableCoins.upper_bound( _keyId ); previous = iterator )
	{
		bool remove = std::find( _previousCoins.begin(), _previousCoins.end(), iterator->second ) != _previousCoins.end();
		iterator++;

		if ( remove )
		{
			m_availableCoins.erase( previous );
		}
		else
		{
			left.push_back( previous->second );
		}
	}

	if ( !CWalletDB(this->strWalletFile).replaceCoins( _keyId, left ) )
		assert( !"fail to write coin" );
}

void CWallet::addCoins( CKeyID const & _keyId, std::vector< CAvailableCoin > const & _coins )
{
	BOOST_FOREACH( CAvailableCoin const & availableCoin, _coins )
	{
		m_availableCoins.insert( std::make_pair( _keyId, availableCoin ) );
	}
}

void CWallet::GetKeyBirthTimes(std::map<CKeyID, int64_t> &mapKeyBirth) const {
    AssertLockHeld(cs_wallet); // mapKeyMetadata
    mapKeyBirth.clear();

    // get birth times for keys with metadata
    for (std::map<CKeyID, CKeyMetadata>::const_iterator it = mapKeyMetadata.begin(); it != mapKeyMetadata.end(); it++)
        if (it->second.nCreateTime)
            mapKeyBirth[it->first] = it->second.nCreateTime;

    // map in which we'll infer heights of other keys
    CBlockIndex *pindexMax = chainActive[std::max(0, chainActive.Height() - 144)]; // the tip can be reorganised; use a 144-block safety margin
    std::map<CKeyID, CBlockIndex*> mapKeyFirstBlock;
    std::set<CKeyID> setKeys;
    GetKeys(setKeys);
    BOOST_FOREACH(const CKeyID &keyid, setKeys) {
        if (mapKeyBirth.count(keyid) == 0)
            mapKeyFirstBlock[keyid] = pindexMax;
    }
    setKeys.clear();

    // if there are no such keys, we're done
    if (mapKeyFirstBlock.empty())
        return;



    // Extract block timestamps for those keys
    for (std::map<CKeyID, CBlockIndex*>::const_iterator it = mapKeyFirstBlock.begin(); it != mapKeyFirstBlock.end(); it++)
        mapKeyBirth[it->first] = it->second->nTime - 7200; // block times can be 2h off
}

bool CWallet::AddDestData(const CTxDestination &dest, const std::string &key, const std::string &value)
{
    if (boost::get<CNoDestination>(&dest))
        return false;

    mapAddressBook[dest].destdata.insert(std::make_pair(key, value));
    if (!fFileBacked)
        return true;
	return CWalletDB(strWalletFile).WriteDestData(CMnemonicAddress(dest).ToString(), key, value);
}

bool CWallet::EraseDestData(const CTxDestination &dest, const std::string &key)
{
    if (!mapAddressBook[dest].destdata.erase(key))
        return false;
    if (!fFileBacked)
        return true;
	return CWalletDB(strWalletFile).EraseDestData(CMnemonicAddress(dest).ToString(), key);
}

bool CWallet::LoadDestData(const CTxDestination &dest, const std::string &key, const std::string &value)
{
    mapAddressBook[dest].destdata.insert(std::make_pair(key, value));
    return true;
}

bool CWallet::GetDestData(const CTxDestination &dest, const std::string &key, std::string *value) const
{
    std::map<CTxDestination, CAddressBookData>::const_iterator i = mapAddressBook.find(dest);
    if(i != mapAddressBook.end())
    {
        CAddressBookData::StringMap::const_iterator j = i->second.destdata.find(key);
        if(j != i->second.destdata.end())
        {
            if(value)
                *value = j->second;
            return true;
        }
    }
    return false;
}

void CWallet::resetDatabase()
{
	LOCK(bitdb.cs_db);
	while (1)
	{
		if (!bitdb.mapFileUseCount.count(strWalletFile) || bitdb.mapFileUseCount[strWalletFile] == 0)
		{
			// Flush log data to the dat file
			bitdb.CloseDb(strWalletFile);
			bitdb.CheckpointLSN(strWalletFile);
			bitdb.mapFileUseCount.erase(strWalletFile);
			bitdb.RemoveDb(strWalletFile);
			CWalletDB(strWalletFile,"cr+");

			mapMasterKeys.clear();
			mapKeys.clear();
			mapScripts.clear();
			m_availableCoins.clear();

			break;
		}
		MilliSleep(100);
	}
}
