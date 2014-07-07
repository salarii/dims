// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_H
#define REQUEST_H

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
		Seed
	};
};

template < class _RequestResponses >
class CMedium;

class CRequestVisitor;

template < class _RequestResponses >
struct CRequest
{
	virtual void accept( CMedium< _RequestResponses > * _medium ) const = 0;
// reconsider this  int
	virtual int getKind() const = 0;
	virtual unsigned int getMediumNumber() const { return -1; }
	virtual ~CRequest(){};
};


}

#endif
