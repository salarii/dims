// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef FAKE_TRACKER_H
#define FAKE_TRACKER_H

namespace monitor
{

class CFakeTracker
{
public:
	//create real tracker 
	CFakeTracker();
	void addInvestigation();
	void checkInvestigationResult();
};

}

#endif