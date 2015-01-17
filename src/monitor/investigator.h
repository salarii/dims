// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef INVESTIGATOR_H
#define INVESTIGATOR_H

namespace monitor
{

struct IvestigationType
{
	enum Enum
	{
		DenialOfService,
		SynchranisationError
	};
};

struct CInvestigationItem
{
	std::list< uint256 > m_trackersId;
	IvestigationType::Enum m_type;
};
/*
ther  may  be  need  for  investigator  to  order  some  proven  tracker  to  proceed  with  investigation

*/


class CInvestigator
{
public:
	void loop();
private:
	std::list< CInvestigationItem > m_investigationQueue;
};

void
CInvestigator::loop()
{
	std::list< CInvestigationItem >::iterator iterator = m_investigationQueue.begin();

	while( iterator != m_investigationQueue.end() )
	{

		iterator++;
	}
}

}

#endif


