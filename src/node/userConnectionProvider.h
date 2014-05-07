// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USER_CONNECTION_PROVIDER_H
#define USER_CONNECTION_PROVIDER_H

#include "common/connectionProvider.h"
#include "configureNodeActionHadler.h"

namespace node
{
class CDummyMedium;

class CUserConnectionProvider : public common::CConnectionProvider< NodeResponses >
{
public:
	CUserConnectionProvider();

	virtual std::list< common::CMedium< NodeResponses > *> provideConnection( int const _actionKind, unsigned _requestedConnectionNumber = -1 );
private:
	CDummyMedium * m_dummyMedium;
};


}

#endif // USER_CONNECTION_PROVIDER_H
