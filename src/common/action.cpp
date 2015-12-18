// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <vector>
#include <boost/foreach.hpp>

#include "common/action.h"
#include "common/request.h"
#include "common/support.h"
#include "common/networkActionRegister.h"

namespace common
{

CAction::CAction( bool _autoDelete )
	: m_inProgress( false )
	, m_executed( false )
	, m_autoDelete( _autoDelete )
	, m_exit( false )
{
	m_actionKey = getRandNumber();

	CNetworkActionRegister::getInstance()->registerServicedByAction( m_actionKey );// this  shouldn't be  here in reality
};

CAction::CAction( uint256 const & _actionKey, bool _autoDelete )
	: m_inProgress( false )
	, m_executed( false )
	, m_autoDelete( _autoDelete )
	, m_exit( false )
{
	m_actionKey = _actionKey;

	CNetworkActionRegister::getInstance()->registerServicedByAction( m_actionKey );// this  shouldn't be  here in reality
};

std::vector< CRequest * >
CAction::getRequests() const
{ return m_requests; }

// following two are  ugly
void
CAction::addRequest( CRequest * _request )
{ m_requests.push_back( _request ); }

uint256
CAction::getActionKey() const
{
	return m_actionKey;
}

void
CAction::forgetRequests()
{
	m_droppedRequests.insert( m_droppedRequests.end(), m_requests.begin(), m_requests.end() );
	m_requests.clear();
}

std::vector< CRequest * > const &
CAction::getDroppedRequests() const
{
	return m_droppedRequests;
}

void
CAction::setInProgress()
{ m_inProgress = true; }

bool
CAction::isInProgress()const
{ return m_inProgress; }

bool
CAction::isExecuted()const
{ return m_executed; }

void
CAction::setExecuted()
{ m_executed = true; }

bool
CAction::autoDelete()
{ return m_autoDelete; }

void
CAction::reset()
{
	m_executed = false;
	m_inProgress = false;
	m_exit =false;
}

void
CAction::setExit()
{ m_exit = true; }

bool
CAction::needToExit()const
{
	if ( m_exit )
	{
		BOOST_FOREACH( CRequest *request, m_droppedRequests )
		{
			if ( !request->isProcessed() )
				return false;
		}

		BOOST_FOREACH( CRequest *request, m_requests )
		{
			if ( !request->isProcessed() )
				return false;
		}

		return true;
	}

	return false;
}

bool
CAction::requestToProcess()const
{
	return !( m_requests.empty() && m_droppedRequests.empty() );
}

CAction::~CAction()
{
	LogPrintf("destructor of action: %p \n", this );
	BOOST_FOREACH( CRequest *request, m_droppedRequests )
	{
		delete request;
	}

	BOOST_FOREACH( CRequest *request, m_requests )
	{
		delete request;
	}

	CNetworkActionRegister::getInstance()->unregisterServicedByAction( m_actionKey );
};

}
