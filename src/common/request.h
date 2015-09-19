// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_H
#define REQUEST_H

#include "uint256.h"

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
	CRequest( CMediumFilter * _mediumFilter = 0 );

	CRequest( uint256 const & _id, CMediumFilter * _mediumFilter = 0 );

	virtual void accept( CMedium * _medium ) const = 0;

	virtual CMediumFilter * getMediumFilter() const;

	uint256
	getId() const;

	virtual ~CRequest();

protected:
	CMediumFilter * m_mediumFilter;

	uint256 m_id;
};

}

#endif
