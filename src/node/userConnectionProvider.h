// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USER_CONNECTION_PROVIDER_H
#define USER_CONNECTION_PROVIDER_H

#include "connectionProvider.h"

namespace node
{
class CDummyMedium;

class CUserConnectionProvider : public CConnectionProvider
{
public:
	CUserConnectionProvider();

	virtual CMedium * provideConnection( RequestKind::Enum const _actionKind );

	virtual std::list< CMedium *> provideConnection( RequestKind::Enum const _actionKind, unsigned _requestedConnectionNumber );
private:
	CDummyMedium * m_dummyMedium;
};


}

#endif // USER_CONNECTION_PROVIDER_H