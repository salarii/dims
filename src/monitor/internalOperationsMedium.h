// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef INTERNAL_OPERATIONS_MEDIUM_H
#define INTERNAL_OPERATIONS_MEDIUM_H

#include "common/medium.h"

namespace monitor
{
/* for now  I am doing  everything in same  thread, do I need to  change  this behavior?? */
class CInternalOperationsMedium : public common::CMedium
{
public:
	virtual bool serviced() const;

	virtual bool flush(){ return true; }

	virtual bool getResponseAndClear( std::multimap< common::CRequest const*, common::DimsResponse > & _requestResponse );

	virtual void add( CConnectToNodeRequest const * _request );

	static CInternalOperationsMedium* getInstance();
		CInternalOperationsMedium();
private:
	void clearResponses();
private:
	std::multimap< common::CRequest const*, common::DimsResponse > m_responses;

	static CInternalOperationsMedium * ms_instance;
};


}



#endif // INTERNAL_OPERATIONS_MEDIUM_H
