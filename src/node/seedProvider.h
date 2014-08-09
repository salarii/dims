// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEED_PROVIDER_H
#define SEED_PROVIDER_H

#include "common/connectionProvider.h"
#include "configureNodeActionHadler.h"

namespace client
{

class CSeedProvider : public common::CConnectionProvider< NodeResponses >
{
public:
	std::list< common::CMedium< NodeResponses > *> provideConnection( common::CMediumFilter< NodeResponses > const & _mediumFilter );

	~CSeedProvider();

	static CSeedProvider * getInstance();

private:
	CSeedProvider();
private:
	static CSeedProvider * ms_instance;
};


}

#endif // SEED_PROVIDER_H
