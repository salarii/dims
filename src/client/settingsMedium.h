// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef	SETTINGS_MEDIUM_H
#define SETTINGS_MEDIUM_H

#include <list>
#include <string>

#include "common/medium.h"
#include "common/responses.h"
#include "common/communicationBuffer.h"

namespace common
{

template < class ResponsesType >
class CContinueReqest;
}


namespace client
{

class CDefaultMedium : public common::CMedium
{
public:

	static CDefaultMedium * getInstance();

	bool serviced() const;

	void add( common::CRequest const * _request );

	void add( CDnsInfoRequest const * _request );

	bool flush();

	bool getResponseAndClear( std::multimap< common::CRequest const*, common::DimsResponse > & _requestResponse );
private:
	void clearResponses();

	CDefaultMedium();

	void getSeedIps( vector<CAddress> & _vAdd );
private:
	static CDefaultMedium * ms_instance;

	std::multimap< common::CRequest const*, common::DimsResponse > m_requestResponse;
};


}

#endif // SETTINGS_MEDIUM_H
