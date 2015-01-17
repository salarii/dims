// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "databaseManager.h"

#include "walletdb.h"

#include "base58.h"
#include "protocol.h"
#include "serialize.h"
#include "sync.h"
#include "wallet.h"

#include <inttypes.h>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/assign.hpp>

using namespace std;
using namespace boost;

namespace common
{

static uint64_t nAccountingEntryNumber = 0;

//
// CIdentificationDB
//
bool
CIdentificationDB::writeKeyForeign( CPubKey const& _vchPubKey)
{
	bool result = Write( std::make_pair((int)Foreign, _vchPubKey.GetID() ), _vchPubKey, false );
	Flush();
	return result;
}


bool
CIdentificationDB::writeKeySelf( CPubKey const& _vchPubKey, CPrivKey const & _vchPrivKey )
{
	std::vector<unsigned char> vchKey;
	vchKey.reserve(_vchPubKey.size() + _vchPrivKey.size());
	vchKey.insert(vchKey.end(), _vchPubKey.begin(), _vchPubKey.end());
	vchKey.insert(vchKey.end(), _vchPrivKey.begin(), _vchPrivKey.end());

	bool result = Write(std::make_pair((int)Self, _vchPubKey), std::make_pair(_vchPrivKey, Hash(vchKey.begin(), vchKey.end())), false);

	Flush();

	return result;
}

bool
CIdentificationDB::eraseKeyForeign( CPubKey const& vchPubKey )
{
	return Erase(std::make_pair( (int)Foreign, vchPubKey.GetID() ));
}

bool
CIdentificationDB::eraseKey( CPubKey const& vchPubKey )
{
	return Erase(std::make_pair( (int)Self, vchPubKey ));
}

class CWalletScanState {
public:
	unsigned int nKeys;
	unsigned int nCKeys;
	unsigned int nKeyMeta;
	bool fIsEncrypted;
	bool fAnyUnordered;
	int nFileVersion;
	vector<uint256> vWalletUpgrade;

	CWalletScanState() {
		nKeys = nCKeys = nKeyMeta = 0;
		fIsEncrypted = false;
		fAnyUnordered = false;
		nFileVersion = 0;
	}
};

bool
ReadKeyValue( std::map< CKeyID, CPubKey > & _keys, CPubKey & _selfKey, CCryptoKeyStore* _keyStore, CDataStream& _ssKey, CDataStream& _ssValue,
			 CWalletScanState &_wss, string& _strErr)
{
	try {
		// Unserialize
		// Taking advantage of the fact that pair serialization
		// is just the two items serialized one after the other
		int strType;
		_ssKey >> strType;

		if (  strType == CIdentificationDB::Foreign)
		{

			CKeyID vchKeyId;
			_ssKey >> vchKeyId;

			CPubKey vchPubKey;

			_ssValue >> vchPubKey;

			if ( vchPubKey.GetID() == vchKeyId )
				return false;

			_keys.insert( std::make_pair( vchKeyId, vchPubKey ) );

		}
		else if ( strType == CIdentificationDB::Self )
		{
			CPubKey vchPubKey;
			_ssKey >> vchPubKey;

			CKey key;
			CPrivKey pkey;
			uint256 hash = 0;

			_wss.nKeys++;
			_ssValue >> pkey;


			if (!vchPubKey.IsValid())
			{
				_strErr = "Error reading wallet database: CPubKey corrupt";
				return false;
			}

			// Old wallets store keys as "key" [pubkey] => [privkey]
			// ... which was slow for wallets with lots of keys, because the public key is re-derived from the private key
			// using EC operations as a checksum.
			// Newer wallets store keys as "key"[pubkey] => [privkey][hash(pubkey,privkey)], which is much faster while
			// remaining backwards-compatible.
			try
			{
				_ssValue >> hash;
			}
			catch(...){}

			bool fSkipCheck = false;

			if (hash != 0)
			{
				// hash pubkey/privkey to accelerate wallet load
				std::vector<unsigned char> vchKey;
				vchKey.reserve(vchPubKey.size() + pkey.size());
				vchKey.insert(vchKey.end(), vchPubKey.begin(), vchPubKey.end());
				vchKey.insert(vchKey.end(), pkey.begin(), pkey.end());

				if (Hash(vchKey.begin(), vchKey.end()) != hash)
				{
					_strErr = "Error reading wallet database: CPubKey/CPrivKey corrupt";
					return false;
				}

				fSkipCheck = true;
			}

			if (!key.Load(pkey, vchPubKey, fSkipCheck))
			{
				_strErr = "Error reading wallet database: CPrivKey corrupt";
				return false;
			}
			if (!_keyStore->AddKeyPubKey(key, vchPubKey))
			{
				_strErr = "Error reading wallet database: LoadKey failed";
				return false;
			}

			_selfKey = vchPubKey;
		}

	} catch (...)
	{
		return false;
	}
	return true;
}

DBErrors CIdentificationDB::loadIdentificationDatabase( std::map< CKeyID, CPubKey > & _keys, CPubKey & _selfKey, CCryptoKeyStore * _store )
{
	CWalletScanState wss;
	bool fNoncriticalErrors = false;
	DBErrors result = DB_LOAD_OK;

	try {
		//LOCK(pwallet->cs_wallet); do  I need  something like this ???
		int nMinVersion = 0;

		// may be  needed  one day
	/*	if (Read((string)"minversion", nMinVersion))
		{
			if (nMinVersion > CLIENT_VERSION)
				return DB_TOO_NEW;
			pwallet->LoadMinVersion(nMinVersion);
		}*/

		// Get cursor
		Dbc* pcursor = GetCursor();
		if (!pcursor)
		{
			LogPrintf("Error getting identification database cursor\n");
			return DB_CORRUPT;
		}

		while (true)
		{
			// Read next record
			CDataStream ssKey(SER_DISK, CLIENT_VERSION);
			CDataStream ssValue(SER_DISK, CLIENT_VERSION);
			int ret = ReadAtCursor(pcursor, ssKey, ssValue);
			if (ret == DB_NOTFOUND)
				break;
			else if (ret != 0)
			{
				LogPrintf("Error reading next record from wallet database\n");
				return DB_CORRUPT;
			}

			// Try to be tolerant of single corrupt records:
			string strErr;
			if (!ReadKeyValue(_keys, _selfKey, _store, ssKey, ssValue, wss, strErr))
			{
				// losing keys is considered a catastrophic error, anything else
				// we assume the user can live with:
				//if (IsKeyType(strType))
					result = DB_CORRUPT;
				/*else
				{
					// Leave other errors alone, if we try to fix them we might make things worse.
					fNoncriticalErrors = true; // ... but do warn the user there is something wrong.
					if (strType == "tx")
						// Rescan if there is a bad transaction record:
						SoftSetBoolArg("-rescan", true);
				}*/
			}
			if (!strErr.empty())
				LogPrintf("%s\n", strErr);
		}
		pcursor->close();
	}
	catch (boost::thread_interrupted) {
		throw;
	}
	catch (...) {
		result = DB_CORRUPT;
	}

	if (fNoncriticalErrors && result == DB_LOAD_OK)
		result = DB_NONCRITICAL_ERROR;

	// Any wallet corruption at all: skip any rewriting or
	// upgrading, we don't want to make it worse.
	if (result != DB_LOAD_OK)
		return result;

	LogPrintf("nFileVersion = %d\n", wss.nFileVersion);

	LogPrintf("Keys: %u plaintext, %u encrypted, %u w/ metadata, %u total\n",
		   wss.nKeys, wss.nCKeys, wss.nKeyMeta, wss.nKeys + wss.nCKeys);

	// nTimeFirstKey is only reliable if all keys have metadata
	//if ((wss.nKeys + wss.nCKeys) != wss.nKeyMeta)
	//	pwallet->nTimeFirstKey = 1; // 0 would be considered 'no value'


	// Rewrite encrypted wallets of versions 0.4.0 and 0.5.0rc:
/*	if (wss.fIsEncrypted && (wss.nFileVersion == 40000 || wss.nFileVersion == 50000))
		return DB_NEED_REWRITE;

	if (wss.nFileVersion < CLIENT_VERSION) // Update
		WriteVersion(CLIENT_VERSION);

	if (wss.fAnyUnordered)
		result = ReorderTransactions(pwallet);
*/
	return result;
}


/*
void ThreadFlushWalletDB(const string& strFile)
{
	// Make this thread recognisable as the wallet flushing thread
	RenameThread("bitcoin-wallet");

	static bool fOneThread;
	if (fOneThread)
		return;
	fOneThread = true;
	if (!GetBoolArg("-flushwallet", true))
		return;

	unsigned int nLastSeen = nWalletDBUpdated;
	unsigned int nLastFlushed = nWalletDBUpdated;
	int64_t nLastWalletUpdate = GetTime();
	while (true)
	{
		MilliSleep(500);

		if (nLastSeen != nWalletDBUpdated)
		{
			nLastSeen = nWalletDBUpdated;
			nLastWalletUpdate = GetTime();
		}

		if (nLastFlushed != nWalletDBUpdated && GetTime() - nLastWalletUpdate >= 2)
		{
			TRY_LOCK(bitdb.cs_db,lockDb);
			if (lockDb)
			{
				// Don't do this if any databases are in use
				int nRefCount = 0;
				map<string, int>::iterator mi = bitdb.mapFileUseCount.begin();
				while (mi != bitdb.mapFileUseCount.end())
				{
					nRefCount += (*mi).second;
					mi++;
				}

				if (nRefCount == 0)
				{
					boost::this_thread::interruption_point();
					map<string, int>::iterator mi = bitdb.mapFileUseCount.find(strFile);
					if (mi != bitdb.mapFileUseCount.end())
					{
						LogPrint("db", "Flushing wallet.dat\n");
						nLastFlushed = nWalletDBUpdated;
						int64_t nStart = GetTimeMillis();

						// Flush wallet.dat so it's self contained
						bitdb.CloseDb(strFile);
						bitdb.CheckpointLSN(strFile);

						bitdb.mapFileUseCount.erase(mi++);
						LogPrint("db", "Flushed wallet.dat %"PRId64"ms\n", GetTimeMillis() - nStart);
					}
				}
			}
		}
	}
}
/*
bool BackupWallet(const CWallet& wallet, const string& strDest)
{
	if (!wallet.fFileBacked)
		return false;
	while (true)
	{
		{
			LOCK(bitdb.cs_db);
			if (!bitdb.mapFileUseCount.count(wallet.strWalletFile) || bitdb.mapFileUseCount[wallet.strWalletFile] == 0)
			{
				// Flush log data to the dat file
				bitdb.CloseDb(wallet.strWalletFile);
				bitdb.CheckpointLSN(wallet.strWalletFile);
				bitdb.mapFileUseCount.erase(wallet.strWalletFile);

				// Copy wallet.dat
				filesystem::path pathSrc = GetDataDir(common::AppType::Tracker) / wallet.strWalletFile;
				filesystem::path pathDest(strDest);
				if (filesystem::is_directory(pathDest))
					pathDest /= wallet.strWalletFile;

				try {
#if BOOST_VERSION >= 104000
					filesystem::copy_file(pathSrc, pathDest, filesystem::copy_option::overwrite_if_exists);
#else
					filesystem::copy_file(pathSrc, pathDest);
#endif
					LogPrintf("copied wallet.dat to %s\n", pathDest.string());
					return true;
				} catch(const filesystem::filesystem_error &e) {
					LogPrintf("error copying wallet.dat to %s - %s\n", pathDest.string(), e.what());
					return false;
				}
			}
		}
		MilliSleep(100);
	}
	return false;
}

//
// Try to (very carefully!) recover wallet.dat if there is a problem.
//
bool CIdentificationDB::Recover(CDBEnv& dbenv, std::string filename, bool fOnlyKeys)
{
	// Recovery procedure:
	// move wallet.dat to wallet.timestamp.bak
	// Call Salvage with fAggressive=true to
	// get as much data as possible.
	// Rewrite salvaged data to wallet.dat
	// Set -rescan so any missing transactions will be
	// found.
	int64_t now = GetTime();
	std::string newFilename = strprintf("wallet.%"PRId64".bak", now);

	int result = dbenv.dbenv.dbrename(NULL, filename.c_str(), NULL,
									  newFilename.c_str(), DB_AUTO_COMMIT);
	if (result == 0)
		LogPrintf("Renamed %s to %s\n", filename, newFilename);
	else
	{
		LogPrintf("Failed to rename %s to %s\n", filename, newFilename);
		return false;
	}

	std::vector<CDBEnv::KeyValPair> salvagedData;
	bool allOK = dbenv.Salvage(newFilename, true, salvagedData);
	if (salvagedData.empty())
	{
		LogPrintf("Salvage(aggressive) found no records in %s.\n", newFilename);
		return false;
	}
	LogPrintf("Salvage(aggressive) found %"PRIszu" records\n", salvagedData.size());

	bool fSuccess = allOK;
	Db* pdbCopy = new Db(&dbenv.dbenv, 0);
	int ret = pdbCopy->open(NULL,               // Txn pointer
							filename.c_str(),   // Filename
							"main",             // Logical db name
							DB_BTREE,           // Database type
							DB_CREATE,          // Flags
							0);
	if (ret > 0)
	{
		LogPrintf("Cannot create database file %s\n", filename);
		return false;
	}
	CWallet dummyWallet;
	CWalletScanState wss;

	DbTxn* ptxn = dbenv.TxnBegin();
	BOOST_FOREACH(CDBEnv::KeyValPair& row, salvagedData)
	{
		if (fOnlyKeys)
		{
			CDataStream ssKey(row.first, SER_DISK, CLIENT_VERSION);
			CDataStream ssValue(row.second, SER_DISK, CLIENT_VERSION);
			string strType, strErr;
			bool fReadOK = ReadKeyValue(&dummyWallet, ssKey, ssValue,
										wss, strType, strErr);
			if (!IsKeyType(strType))
				continue;
			if (!fReadOK)
			{
				LogPrintf("WARNING: CIdentificationDB::Recover skipping %s: %s\n", strType, strErr);
				continue;
			}
		}
		Dbt datKey(&row.first[0], row.first.size());
		Dbt datValue(&row.second[0], row.second.size());
		int ret2 = pdbCopy->put(ptxn, &datKey, &datValue, DB_NOOVERWRITE);
		if (ret2 > 0)
			fSuccess = false;
	}
	ptxn->commit(0);
	pdbCopy->close(0);
	delete pdbCopy;

	return fSuccess;
}

bool CIdentificationDB::Recover(CDBEnv& dbenv, std::string filename)
{
	return CIdentificationDB::Recover(dbenv, filename, false);
}
*/




}
