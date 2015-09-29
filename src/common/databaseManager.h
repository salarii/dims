// Copyright (c) 2014-2015 DiMS dev-team
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
		Foreign,
		Self
	};
public:
	CIdentificationDB(std::string strFilename, const char* pszMode="r+") : CDB(strFilename.c_str(), pszMode)
	{
	}
	bool writeKeyForeign( CPubKey const& _vchPubKey);

	bool writeKeySelf( CPubKey const& _vchPubKey, CPrivKey const & _vchPrivKey );

	bool eraseKey( CPubKey const& vchPubKey );

	bool eraseKeyForeign( CPubKey const& vchPubKey );

	DBErrors loadIdentificationDatabase( std::map< CKeyID, CPubKey > & _keys, CPubKey & _selfKey, CCryptoKeyStore * _store );
private:
	CIdentificationDB(const CIdentificationDB&);
	void operator=(const CIdentificationDB&);
private:
	static std::map< CIdentificationDB::Enum, std::string > ms_enumToMnemonic;
};

}

#endif // DATABASE_MANAGER_H
