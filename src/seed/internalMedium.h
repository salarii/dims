#ifndef INTERNAL_MEDIUM_H
#define INTERNAL_MEDIUM_H

#include "common/medium.h"

namespace common
{
class CConnectToNodeRequest;
}

namespace seed
{

class CInternalMedium : public common::CMedium
{
public:
	virtual bool serviced() const;

	virtual bool flush(){ return true; }

	virtual bool getResponseAndClear( std::multimap< common::CRequest const*, common::DimsResponse > & _requestResponse );

	virtual void add( common::CConnectToNodeRequest const *_request );

	static CInternalMedium* getInstance();
private:
	void clearResponses();

	CInternalMedium();
private:
	std::multimap< common::CRequest const*, common::DimsResponse > m_responses;

	static CInternalMedium * ms_instance;
};


}

#endif // INTERNALMEDIUM_H
