#ifndef INTERNAL_MEDIUM_H
#define INTERNAL_MEDIUM_H

#include "configureSeedActionHandler.h"
#include "common/medium.h"

namespace common
{
template < class RequestType >
class CConnectToNodeRequest;
}

namespace seed
{

class CInternalMedium : public common::CMedium< SeedResponses >
{
public:
	virtual bool serviced() const;

	virtual bool flush(){ return true; }

	virtual bool getResponse( std::vector< SeedResponses > & _requestResponse ) const;

	virtual void clearResponses();

	virtual void add( common::CConnectToNodeRequest< SeedResponses > const *_request );

	static CInternalMedium* getInstance();
private:
	CInternalMedium();
private:
	std::vector< SeedResponses > m_responses;

	static CInternalMedium * ms_instance;
};


}

#endif // INTERNALMEDIUM_H
