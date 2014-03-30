#ifndef _INFORMATION_PROVIDER_H
#define _INFORMATION_PROVIDER_H

#include <string>

#include <map>
#include <list>
#include "monitorsScaner_types.h"

namespace monitorsScaner
{
class Data;
}


class CInforamtionProvider
{
public:
	CInforamtionProvider();

	void getInfo(monitorsScaner::Data& _return, const monitorsScaner::InfoRequest& infoRequest);

	void changeStorage();

	void changeStorageThread();
private:
	void getTrackers(monitorsScaner::Data& _trackers, std::string _publicKey ) const;
	void getMonitorsInfo(monitorsScaner::Data& _monitors)const;
private:
	std::list<std::list<std::string> > vec1;
	std::list<std::list<std::string> > vec2;

	std::list<std::list<std::string> > vec3;
	std::list<std::list<std::string> > vec4;

	std::list<std::list<std::string> > * usedVectorTest;

	std::list<std::list<std::string> > * usedVectorMain;

	std::list<std::list<std::string> > * usedVector;

	std::multimap< std::string,std::list<std::string> > m_trackers;


};


#endif

