// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTION_H
#define ACTION_H

#include <vector>
#include <boost/foreach.hpp>

#include "common/support.h"
#include "common/networkActionRegister.h"

namespace common
{

class CSetResponseVisitor;

class CRequest;

class CAction
{
public:
	CAction( bool _autoDelete = true ): m_inProgress( false ), m_executed( false ), m_autoDelete( _autoDelete ), m_exit( false )
	{
		m_actionKey = getRandNumber();

		CNetworkActionRegister::getInstance()->registerServicedByAction( m_actionKey );// this  shouldn't be  here in reality
	};

	CAction( uint256 const & _actionKey, bool _autoDelete = true ): m_inProgress( false ), m_executed( false ), m_autoDelete( _autoDelete ), m_exit( false )
	{
		m_actionKey = _actionKey;

		CNetworkActionRegister::getInstance()->registerServicedByAction( m_actionKey );// this  shouldn't be  here in reality
	};

	virtual void accept( CSetResponseVisitor & _visitor ) = 0;

	virtual std::vector< CRequest * > getRequests() const{ return m_requests; }

	// following two are  ugly
	virtual void addRequest( CRequest * _request ){ m_requests.push_back( _request ); }

	uint256
	getActionKey() const
	{
		return m_actionKey;
	}

	virtual void forgetRequests()
	{
		m_droppedRequests.insert( m_droppedRequests.end(), m_requests.begin(), m_requests.end() );
		m_requests.clear();
	}
	std::vector< CRequest * > const & getDroppedRequests() const
	{
		return m_droppedRequests;
	}
	void setInProgress(){ m_inProgress = true; }

	bool isInProgress()const{ return m_inProgress; }

	bool isExecuted()const{ return m_executed; }

	void setExecuted(){ m_executed = true; }

	bool autoDelete(){ return m_autoDelete; }

	virtual void reset(){ m_executed = false; m_inProgress = false; m_exit =false; }

	void setExit(){ m_exit = true; }

	bool needToExit()const{ return m_exit; }

	bool requestToProcess()const
	{
		return !( m_requests.empty() && m_droppedRequests.empty() );
	}

	virtual ~CAction()
	{
		BOOST_FOREACH( CRequest *request, m_droppedRequests )
		{
			delete request;
		}

		CNetworkActionRegister::getInstance()->unregisterServicedByAction( m_actionKey );
	};
protected:
	bool m_inProgress;

	bool m_executed;

	bool const m_autoDelete;

	bool m_exit;

	uint256 m_actionKey;

	std::vector< CRequest * > m_requests;

	std::vector< CRequest * > m_droppedRequests;
};

}

#endif
