// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_H
#define REQUEST_H

#include "common/support.h"

struct CBufferAsStream;

namespace common
{

struct RequestKind
{
	enum Enum
	{
		NetworkInfo = 0,
		Transaction,
		TransactionStatus,
		Balance,
		Seed,
		UndeterminedTrackers,
		Trackers,
		Monitors,
		Time,
		Unknown
	};
};

class CMedium;

class CMediumFilter;

class CRequestVisitor;

class CRequest
{
public:
	CRequest( CMediumFilter * _mediumFilter = 0 ):m_mediumFilter( _mediumFilter )
	{
		m_id = getRandNumber();
	}

	CRequest( uint256 const & _id, CMediumFilter * _mediumFilter = 0 ) : m_mediumFilter( _mediumFilter ), m_id( _id )
	{
	}

	virtual void accept( CMedium * _medium ) const = 0;

	virtual CMediumFilter * getMediumFilter() const{ return m_mediumFilter; }

	uint256
	getId() const
	{
		return m_id;
	}

	virtual ~CRequest()
	{
		if ( m_mediumFilter )
			delete m_mediumFilter;
	};

protected:
	CMediumFilter * m_mediumFilter;

	uint256 m_id;
};

}

#endif
