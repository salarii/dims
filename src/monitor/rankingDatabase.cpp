// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "serialize.h"
#include "sync.h"
#include "util.h"

#include <inttypes.h>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/assign.hpp>

#include "monitor/reputationTracer.h"
#include "monitor/rankingDatabase.h"

namespace monitor
{

using namespace std;
using namespace boost;

CRankingDatabase * CRankingDatabase::ms_instance = NULL;

CRankingDatabase*
CRankingDatabase::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CRankingDatabase( "rankingData", "rc+" );
	};
	return ms_instance;
}


//
// CRankingDatabase
//
bool
CRankingDatabase::writeTrackerData( common::CTrackerData const& _trackerData )
{

	if( Exists(std::make_pair( std::string("tracker"), _trackerData.m_publicKey.GetID() )) )
		eraseTrackerData( _trackerData.m_publicKey );

	bool result = Write( std::make_pair( std::string("tracker"), _trackerData.m_publicKey.GetID() ), _trackerData, false );
	Flush();
	return result;
}

bool
CRankingDatabase::eraseTrackerData( CPubKey const & _publicKey )
{
	return Erase(std::make_pair( std::string("tracker"), _publicKey.GetID() ));
}


bool
ReadTrackerData( std::map< uint160, common::CTrackerData > & _trackerData, CDataStream& _stream, CDataStream& _ssValue, string& _strErr)
{
	try {
		// Unserialize
		// Taking advantage of the fact that pair serialization
		// is just the two items serialized one after the other
		std::string type;
		_stream >> type;

		if ( type == "tracker" )
		{

			CKeyID keyId;
			_stream >> keyId;

			common::CTrackerData trackerData;

			_ssValue >> trackerData;

			if ( trackerData.m_publicKey.GetID() != keyId )
				return false;

			_trackerData.insert( std::make_pair( keyId, trackerData ) );

		}
	} catch (...)
	{
		return false;
	}
	return true;
}

DBErrors CRankingDatabase::loadIdentificationDatabase( std::map< uint160, common::CTrackerData > & _trackers )
{
	bool fNoncriticalErrors = false;
	DBErrors result = DB_LOAD_OK;

	try {
		//LOCK(pwallet->cs_wallet); do  I need  something like this ???
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
				LogPrintf("Error reading next record from tracker ranking database\n");
				return DB_CORRUPT;
			}

			// Try to be tolerant of single corrupt records:
			string strErr;
			if ( !ReadTrackerData( _trackers, ssKey, ssValue, strErr ) )
			{
					result = DB_CORRUPT;

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

	return result;
}

void
CRankingDatabase::resetDb()
{
	bitdb.CloseDb("rankingData");
	bitdb.CheckpointLSN("rankingData");
	bitdb.mapFileUseCount.erase("rankingData");
	bitdb.RemoveDb("rankingData");
	delete getInstance();
	getInstance();
}

}
