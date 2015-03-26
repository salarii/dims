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

class CInternalMedium : public common::CSeedBaseMedium
{
public:
	virtual bool serviced() const;

	virtual bool flush(){ return true; }

	virtual bool getResponseAndClear( std::multimap< common::CRequest< common::CSeedTypes >const*, SeedResponses, common::CLess< common::CRequest< common::CSeedTypes > > > & _requestResponse );

	virtual void add( common::CConnectToNodeRequest< common::CSeedTypes > const *_request );

	static CInternalMedium* getInstance();
private:
	void clearResponses();

	CInternalMedium();
private:
	std::multimap< common::CRequest< common::CSeedTypes >const*, SeedResponses, common::CLess< common::CRequest< common::CSeedTypes > > > m_responses;

	static CInternalMedium * ms_instance;
};


}

#endif // INTERNALMEDIUM_H
