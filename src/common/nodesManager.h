// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODES_MANAGER_H
#define NODES_MANAGER_H

#include "common/connectionProvider.h"
#include "common/selfNode.h"
#include "common/nodeMedium.h"
#include "common/manageNetwork.h"
#include "common/mediumKinds.h"

namespace common
{
// class common::CConnectionProvider is problematic, may providers make  things confusing, sometimes it is not  clear which provider is responsible for  mediums of specific

class CNodesManager : public common::CConnectionProvider
{
public:
	struct CCustomAddressComparator // ugly but port may change
	{
		bool operator()( const CAddress& _lhs, const CAddress& _rhs) const
		{
			return _lhs.ToStringIP() < _rhs.ToStringIP();
		}
	};
public:
	bool getMessagesForNode( common::CSelfNode * _node, std::vector< common::CMessage > & _messages );

	bool processMessagesFromNode( common::CSelfNode * _node, std::vector< common::CMessage > const & _messages );

	void addNode( CNodeMedium * _medium );

	std::list< CMedium *> provideConnection( CMediumFilter const & _mediumFilter );

	virtual CNodeMedium* getMediumForNode( common::CSelfNode * _node ) const;

	CMedium * findNodeMedium( uintptr_t _ptr ) const;

		virtual void eraseMedium( uintptr_t _nodePtr );

	virtual std::list< CMedium *> getNodesByClass( CMediumKinds::Enum _nodesClass ) const = 0;

	bool getAddress( uintptr_t _nodePtr, CAddress & _address ) const;

	static CNodesManager * getInstance()
	{
		return ms_instance;
	}

	void setPublicKey( CAddress const & _address, CPubKey const & _pubKey );

	bool getPublicKey( CAddress const & _address, CPubKey & _pubKey ) const;

protected:
	CNodesManager();

	std::map< CAddress, CPubKey, CCustomAddressComparator > m_keyStore;

	mutable boost::mutex m_nodesLock;

	std::map< uintptr_t, CNodeMedium* > m_ptrToNodes;
protected:
	static CNodesManager * ms_instance;
};

}

#endif // NODES_MANAGER_H
