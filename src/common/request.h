// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_H
#define REQUEST_H

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

template < class _RequestResponses >
struct CRequest
{
	CRequest( common::CMediumFilter< _RequestResponses > * _mediumFilter = 0 ):m_mediumFilter( _mediumFilter ){}

	virtual void accept( CMedium< _RequestResponses > * _medium ) const = 0;

	virtual CMediumFilter< _RequestResponses > * getMediumFilter() const{ return m_mediumFilter; }

	virtual ~CRequest()
	{
		if ( m_mediumFilter )
			delete m_mediumFilter;
	};

	common::CMediumFilter< _RequestResponses > * m_mediumFilter;
};


}

#endif
