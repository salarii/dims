// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef	MEDIUM_H
#define MEDIUM_H

#include "errorRespond.h"
#include "requestResponse.h"
#include <vector>
#include "visitorConfigurationUtilities.h"

// this is  weak point of  this implementation I don't know  how to get rid of this at the moment
#include "tracker/trackerRequestsList.h"

namespace common
{

template < class _RequestResponses >
struct CRequest;

class CMediumException : public std::exception
{
public:
    CMediumException(ErrorType::Enum _error):m_error(_error){};
    ErrorType::Enum m_error;
};

template < class _RequestResponses >
class CMedium
{
public:
	virtual bool serviced() const = 0;
	virtual bool flush() = 0;
	virtual void clearResponses() = 0;
	virtual bool getResponse( std::vector< _RequestResponses > & _requestResponse ) const = 0;
	virtual void add( CRequest< _RequestResponses > const * _request ) = 0;
	virtual ~CMedium(){};
};

template <>
class CMedium< tracker::TrackerResponses >
{
public:
	virtual bool serviced() const = 0;
	virtual bool flush() = 0;

	virtual bool getResponse( std::vector< tracker::TrackerResponses > & _requestResponse ) const = 0;
	virtual void clearResponses() = 0;
	virtual void add( VisitorParam( tracker::TrackerRequestsList ,0 ) const * _request ){};
	virtual void add( VisitorParam( tracker::TrackerRequestsList ,1 ) const * _request ){};
	virtual void add( VisitorParam( tracker::TrackerRequestsList ,2 ) const * _request ){};
	virtual void add( VisitorParam( tracker::TrackerRequestsList ,3 ) const * _request ){};
	virtual void add( VisitorParam( tracker::TrackerRequestsList ,4 ) const * _request ){};
	virtual void add( VisitorParam( tracker::TrackerRequestsList ,5 ) const * _request ){};
	virtual void add( VisitorParam( tracker::TrackerRequestsList ,6 ) const * _request ){};
	virtual void add( VisitorParam( tracker::TrackerRequestsList ,7 ) const * _request ){};
	virtual void add( VisitorParam( tracker::TrackerRequestsList ,8 ) const * _request ){};
	virtual void add( VisitorParam( tracker::TrackerRequestsList ,9 ) const * _request ){};
	virtual void add( VisitorParam( tracker::TrackerRequestsList ,10 ) const * _request ){};
	virtual ~CMedium(){};
};


}

#endif // MEDIUM_H
