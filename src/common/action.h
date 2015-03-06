// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTION_H
#define ACTION_H

#include <vector>
#include <boost/foreach.hpp>

namespace common
{

template < class _Type >
class CSetResponseVisitor;

template < class _Type > struct CRequest;


template < class _Type >
class CAction
{
public:
	CAction( bool _autoDelete = true ): m_executed( false ), m_inProgress( false ), m_autoDelete( _autoDelete ){};

	virtual void accept( CSetResponseVisitor< _Type > & _visitor ) = 0;

	virtual std::vector< CRequest< _Type >* > getRequests() const{ return m_requests; }

	// following two are  ugly
	virtual void addRequests( CRequest< _Type >* _request ){ m_requests.push_back( _request ); }

	virtual void dropRequests()
	{
		m_droppedRequests.insert( m_droppedRequests.end(), m_requests.begin(), m_requests.end() );
		m_requests.clear();
	}

	void setInProgress(){ m_inProgress = true; }

	bool isInProgress()const{ return m_inProgress; }

	bool isExecuted()const{ return m_executed; }

	void setExecuted(){ m_executed = true; }

	bool autoDelete(){ return m_autoDelete; }

	virtual void reset(){ m_executed = false; m_inProgress = false; }

	virtual ~CAction()
	{
		BOOST_FOREACH( CRequest< _Type >*request, m_droppedRequests )
		{
			delete request;
		}
	};
protected:
	bool m_inProgress;

	bool m_executed;

	bool const m_autoDelete;

	std::vector< CRequest< _Type >* > m_requests;

	std::vector< CRequest< _Type >* > m_droppedRequests;
};

}

#endif
