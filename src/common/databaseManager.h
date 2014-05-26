// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>

#include "db.h"
#include "key.h"

class CCryptoKeyStore;

namespace common
{

enum DBErrors
{
	DB_LOAD_OK,
	DB_CORRUPT,
	DB_NONCRITICAL_ERROR,
	DB_TOO_NEW,
	DB_LOAD_FAIL,
	DB_NEED_REWRITE
};


class CIdentificationDB : public CDB
{
public:
	enum Enum
	{
		Monitor,
		Tracker,
		Self
	};
public:
	CIdentificationDB(std::string strFilename, const char* pszMode="r+") : CDB(strFilename.c_str(), pszMode)
	{
	}
	bool writeKey( CPubKey const& vchPubKey, CPrivKey const & vchPrivKey, CIdentificationDB::Enum const _type);

	bool eraseKey( CPubKey const& vchPubKey, CPrivKey const & vchPrivKey, CIdentificationDB::Enum const _type);
//	bool WriteCryptedKey(const CPubKey& vchPubKey, const std::vector<unsigned char>& vchCryptedSecret, const CKeyMetadata &keyMeta);
//	bool WriteMasterKey(unsigned int nID, const CMasterKey& kMasterKey);

/*
	bool ReadPool(int64_t nPool, CKeyPool& keypool);
	bool WritePool(int64_t nPool, const CKeyPool& keypool);
	bool ErasePool(int64_t nPool);

	bool WriteMinVersion(int nVersion);
*/
	DBErrors loadIdentificationDatabase( std::multimap< CIdentificationDB::Enum, CKeyID > & _indicator, CCryptoKeyStore * _store );
private:
	CIdentificationDB(const CIdentificationDB&);
	void operator=(const CIdentificationDB&);

	static std::string transalteToMnemonic( CIdentificationDB::Enum const _enum);
	//static bool Recover(CDBEnv& dbenv, std::string filename, bool fOnlyKeys);
	//static bool Recover(CDBEnv& dbenv, std::string filename);
private:
	static std::map< CIdentificationDB::Enum, std::string > ms_enumToMnemonic;
};

}

#endif // DATABASE_MANAGER_H
