// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef INTERNAL_OPERATIONS_MEDIUM_H
#define INTERNAL_OPERATIONS_MEDIUM_H

#include "common/medium.h"
#include "configureMonitorActionHandler.h"

namespace monitor
{
/* for now  I am doing  everything in same  thread, do I need to  change  this behavior?? */
class CInternalOperationsMedium : public common::CMedium< MonitorResponses >
{
public:
	virtual bool serviced() const;

	virtual bool flush(){ return true; }

	virtual bool getResponseAndClear( std::map< common::CRequest< MonitorResponses >*, std::vector< MonitorResponses > > & _requestResponse );

	virtual void add( CConnectToNodeRequest const * _request );

	virtual void add( common::CContinueReqest<MonitorResponses> const * _request );

	static CInternalOperationsMedium* getInstance();
		CInternalOperationsMedium();
private:
	void clearResponses();
private:
	std::vector< MonitorResponses > m_responses;

	static CInternalOperationsMedium * ms_instance;
};


}



#endif // INTERNAL_OPERATIONS_MEDIUM_H
