#ifndef _INFORMATION_PROVIDER_H
#define _INFORMATION_PROVIDER_H


#include <boost/assign/std/vector.hpp>
#include <string>

#include <map>
#include "boost/assign.hpp"

class CInforamtionProvider
{
public:
	CInforamtionProvider();

	void getInfo(Data& _return, const InfoRequest& infoRequest) const;

	void changeStorage();

	void changeStorageThread();
private:
	void getTrackers(Data& _trackers);
	void getMonitorsInfo(Data& _monitors);
private:
	std::vector<std::vector<std::string> > vec1;
	std::vector<std::vector<std::string> > vec2;

	std::vector<std::vector<std::string> > vec3;
	std::vector<std::vector<std::string> > vec4;

	std::vector<std::vector<std::string> > * usedVectorTest;

	std::vector<std::vector<std::string> > * usedVectorMain; 

	std::vector<std::vector<std::string> > * usedVector; 

	std::multimap< std::string,std::vector<std::string> > m_trackers;


}


#endif

