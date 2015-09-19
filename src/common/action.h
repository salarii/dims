// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTION_H
#define ACTION_H

#include <vector>

#include "uint256.h"

namespace common
{

class CSetResponseVisitor;

class CRequest;

class CAction
{
public:
	CAction( bool _autoDelete = true );

	CAction( uint256 const & _actionKey, bool _autoDelete = true );

	virtual void accept( CSetResponseVisitor & _visitor ) = 0;

	virtual std::vector< CRequest * > getRequests() const;

	virtual void addRequest( CRequest * _request );

	uint256
	getActionKey() const;

	virtual void forgetRequests();

	std::vector< CRequest * > const & getDroppedRequests() const;

	void setInProgress();

	bool isInProgress() const;

	bool isExecuted() const;

	void setExecuted();

	bool autoDelete();

	virtual void reset();

	void setExit();

	bool needToExit() const;

	bool requestToProcess() const;

	virtual ~CAction();
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
