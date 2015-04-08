// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_H
#define REQUEST_H

#include "types.h"

struct CBufferAsStream;

namespace common
{

// this is obsolete
// soon  need to be replaced by something else
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

template < class _RequestResponses >
class CMedium;

template < class _RequestResponses >
class CMediumFilter;

class CRequestVisitor;

template < class _Types >
class CRequest
{
public:
	typedef typename _Types::Medium MediumType;
	typedef typename _Types::Filter FilterType;
public:
	CRequest( FilterType * _mediumFilter = 0 ):m_mediumFilter( _mediumFilter )
	{
		m_key = getRandNumber();
	}

	virtual void accept( MediumType * _medium ) const = 0;

	virtual FilterType * getMediumFilter() const{ return m_mediumFilter; }

	uint256
	getKey() const
	{
		return m_key;
	}

	virtual ~CRequest()
	{
		if ( m_mediumFilter )
			delete m_mediumFilter;
	};

protected:
	FilterType * m_mediumFilter;

	uint256 m_key;
};


}

#endif
