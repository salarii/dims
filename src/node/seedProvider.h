// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEED_PROVIDER_H
#define SEED_PROVIDER_H

#include "connectionProvider.h"

namespace node
{

class CSeedProvider : public CConnectionProvider
{
public:
	CMedium * provideConnection( RequestKind::Enum const _actionKind );
	// this  will be  rather  complex  stuff  leave  it  for  better  times
	virtual std::list< CMedium *> provideConnection( RequestKind::Enum const _actionKind, unsigned _requestedConnectionNumber );

	~CSeedProvider();

	static CSeedProvider * getInstance();

private:
	CSeedProvider();
private:
	static CSeedProvider * ms_instance;
};


}

#endif // SEED_PROVIDER_H