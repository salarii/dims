// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MONITOR_REQUESTS_H
#define MONITOR_REQUESTS_H

#include "common/request.h"
#include "configureMonitorActionHandler.h"

namespace monitor
{

class CConnectToNodeRequest : public common::CRequest< common::CMonitorTypes >
{
public:
	CConnectToNodeRequest( std::string const & _address, CAddress const & _serviceAddress );

	virtual void accept( common::CMonitorBaseMedium * _medium ) const;

	std::string getAddress() const;

	CAddress getServiceAddress() const;
private:
	std::string const m_address;

	CAddress const m_serviceAddress;
};

class CRegistrationTerms : public common::CRequest< common::CMonitorTypes >
{
public:
	CRegistrationTerms( uint256 const & _actionKey,  unsigned int _price, int64_t const & _period , common::CMonitorMediumFilter * _mediumFilter );

	virtual void accept( common::CMonitorBaseMedium * _medium ) const;

	unsigned int getPrice() const
	{
		return m_price;
	}

	int64_t const & getPeriod() const
	{
		return m_period;
	}

	uint256 const & getActionKey() const
	{
		return m_actionKey;
	}
private:
	unsigned int m_price;

	int64_t m_period;

	uint256 const m_actionKey;
};

class CInfoRequest : public common::CRequest< common::CMonitorTypes >
{
public:
	CInfoRequest( uint256 const & _actionKey, common::CMonitorMediumFilter * _mediumFilter );

	virtual void accept( common::CMonitorBaseMedium * _medium ) const;

	uint256 const & getActionKey() const
	{
		return m_actionKey;
	}
private:
	uint256 const m_actionKey;
};


}


#endif // MONITOR_REQUESTS_H
