// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef RANKINGDATABASE_H
#define RANKINGDATABASE_H

#include <string>

#include "db.h"

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

struct CTrackerData;

class CRankingDatabase : public CDB
{
public:
	enum Enum
	{
		Foreign,
		Self
	};
public:
	bool writeTrackerData( CTrackerData const& _trackerData );

	bool eraseTrackerData( CPubKey const & _publicKey );

	DBErrors loadIdentificationDatabase( std::map< uint160, CTrackerData > & _trackers );

	static CRankingDatabase * getInstance();
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
