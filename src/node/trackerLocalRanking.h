// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_LOCAL_RANKING_H
#define TRACKER_LOCAL_RANKING_H

#include <set>
#include <QString>
#include <functional> 

namespace node
{

class CNetworkClient;

struct CTrackerStats
{
	QString m_publicKey;
	unsigned int  m_reputation;
	float m_price;
	QString m_ip;
	unsigned int m_port;

};

struct CompareTracker : public std::binary_function<CTrackerStats ,CTrackerStats ,bool>
{
	bool operator() ( CTrackerStats const & _tracker1, CTrackerStats const & _tracker2) const
	{
		return true;
	}
};

class CTrackerLocalRanking
{
public:
	CNetworkClient * connect();
	float getPrice();
private:
	std::set< CTrackerStats, CompareTracker > m_ranking;
};




}

#endif // TRACKER_LOCAL_RANKING_H