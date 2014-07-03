// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef INTERNAL_OPERATIONS_MEDIUM_H
#define INTERNAL_OPERATIONS_MEDIUM_H

#include "common/medium.h"
#include "configureTrackerActionHandler.h"

namespace tracker
{
/* for now  I am doing  everything in same  thread, do I need to  change  this behavior?? */
class CInternalOperationsMedium : public common::CMedium< TrackerResponses >
{
public:
	virtual bool serviced() const;

	virtual bool flush(){ return true; }

	virtual bool getResponse( std::vector< TrackerResponses > & _requestResponse ) const;

	virtual void clearResponses();

	virtual void add(CGetBalanceRequest const * _request );

	virtual void add(CValidateTransactionsRequest const * _request );

	virtual void add( CConnectToTrackerRequest const *_request );

	virtual void add( common::CContinueReqest<TrackerResponses> const * _request );

	static CInternalOperationsMedium* getInstance();
		CInternalOperationsMedium();
private:
	std::vector< TrackerResponses > m_trackerResponses;

	static CInternalOperationsMedium * ms_instance;
};


}



#endif // INTERNAL_OPERATIONS_MEDIUM_H
