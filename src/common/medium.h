// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef	MEDIUM_H
#define MEDIUM_H

#include "errorRespond.h"
#include "requestResponse.h"
#include <vector>

namespace common
{

struct CRequest;

class CMediumException : public std::exception
{
public:
    CMediumException(ErrorType::Enum _error):m_error(_error){};
    ErrorType::Enum m_error;
};

class CMedium
{
public:
	virtual bool serviced() const = 0;
	virtual bool flush() = 0;
	virtual bool getResponse( std::vector< RequestResponse > & _requestResponse ) const = 0;
protected:
	virtual void add( CRequest const * _request ) = 0;
};


}

#endif // MEDIUM_H
