// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef _INFORMATION_PROVIDER_H
#define _INFORMATION_PROVIDER_H

#include <string>

#include <map>
#include <list>
#include "gen-cpp/monitorsScaner_types.h"

namespace monitorsScaner
{
class Data;
}


class CInforamtionProvider
{
public:
	static CInforamtionProvider* getInstance();

	void getInfo(monitorsScaner::Data& _return, const monitorsScaner::InfoRequest& infoRequest);

	void reloadData();

	void reloadThread();
private:
	CInforamtionProvider();

	void getTrackers(monitorsScaner::Data& _trackers, std::string _publicKey ) const;

	void getMonitorsInfo(monitorsScaner::Data& _monitors)const;
private:
	static CInforamtionProvider * ms_instance;

	std::list<std::list<std::string> > m_usedMonitorsTest;

	std::list<std::list<std::string> > m_usedMonitorsMain;

	std::list<std::list<std::string> > m_usedMonitors;

	std::multimap< std::string,std::list<std::string> > m_trackers;
};


#endif

