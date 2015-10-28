// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MONITOR_REQUESTS_H
#define MONITOR_REQUESTS_H

#include "protocol.h"

#include "common/request.h"

namespace monitor
{

class CConnectToNodeRequest : public common::CRequest
{
public:
	CConnectToNodeRequest( std::string const & _address, CAddress const & _serviceAddress );

	virtual void accept( common::CMedium * _medium ) const;

	std::string getAddress() const;

	CAddress getServiceAddress() const;
private:
	std::string const m_address;

	CAddress const m_serviceAddress;
};

class CInfoRequest : public common::CRequest
{
public:
	CInfoRequest( uint256 const & _actionKey, common::CMediumFilter * _mediumFilter );

	virtual void accept( common::CMedium * _medium ) const;

	uint256 const & getActionKey() const
	{
		return m_actionKey;
	}
private:
	uint256 const m_actionKey;
};


}


#endif // MONITOR_REQUESTS_H
