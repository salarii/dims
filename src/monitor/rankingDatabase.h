// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef RANKINGDATABASE_H
#define RANKINGDATABASE_H

#include <string>

#include "db.h"

#include "common/struct.h"

namespace monitor
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

class CRankingDatabase : public CDB
{
public:
	enum Enum
	{
		Foreign,
		Self
	};
public:
	bool writeTrackerData( common::CTrackerData const& _trackerData );

	bool eraseTrackerData( CPubKey const & _publicKey );

	DBErrors loadIdentificationDatabase( std::map< uint160, common::CTrackerData > & _trackers );

	static CRankingDatabase * getInstance();

	static void resetDb();
private:
	CRankingDatabase(std::string strFilename, const char* pszMode="r+") : CDB(strFilename.c_str(), pszMode)
	{
	}
private:
	static CRankingDatabase * ms_instance;

	CRankingDatabase( CRankingDatabase const &);

	void operator=( CRankingDatabase const &);
};

}

#endif // RANKINGDATABASE_H
