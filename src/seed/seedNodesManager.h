// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef SEED_NODES_MANAGER_H
#define SEED_NODES_MANAGER_H

#include "common/nodesManager.h"
#include "common/communicationProtocol.h"
#include "common/connectionProvider.h"

namespace seed
{

class CSeedNodeMedium;

class CSeedNodesManager : public common::CNodesManager
{
public:
	static CSeedNodesManager * getInstance();

	std::list< common::CMedium *> provideConnection( common::CMediumFilter const & _mediumFilter );

	void setPublicKey( uintptr_t _nodeIndicator, CPubKey const & _pubKey );

	bool getPublicKey( uintptr_t _nodeIndicator, CPubKey & _pubKey ) const;

	bool clearPublicKey( uintptr_t _nodeIndicator );

	bool isKnown( CPubKey const & _pubKey ) const;

	std::list< common::CMedium *> getInternalMedium();

	std::list< common::CMedium *> getNodesByClass( common::CMediumKinds::Enum _nodesClass ) const{ return std::list< common::CMedium *>(); }// not used  right now
private:
	CSeedNodesManager();
private:
	std::map< uintptr_t, CPubKey > m_keyStore;
};

}

#endif // SEED_NODES_MANAGER_H
