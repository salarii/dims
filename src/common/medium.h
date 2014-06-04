// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef	MEDIUM_H
#define MEDIUM_H

#include "requestResponse.h"

#include "visitorConfigurationUtilities.h"

// this is  weak point of  this implementation I don't know  how to get rid of this at the moment
#include "tracker/trackerRequestsList.h"
#include "node/nodeRequestList.h"
#include "node/configureNodeActionHadler.h"
#include "monitor/configureMonitorActionHandler.h"
#include "monitor/monitorRequestsList.h"

namespace common
{

template < class _RequestResponses >
struct CRequest;

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

template <>
class CMedium< node::NodeResponses >
{
public:
	virtual bool serviced() const = 0;
	virtual bool flush() = 0;

	virtual bool getResponse( std::vector< node::NodeResponses > & _requestResponse ) const = 0;
	virtual void clearResponses() = 0;
	virtual void add( VisitorParam( node::NodeRequestsList ,0 ) const * _request ){};
	virtual void add( VisitorParam( node::NodeRequestsList ,1 ) const * _request ){};
	virtual void add( VisitorParam( node::NodeRequestsList ,2 ) const * _request ){};
	virtual void add( VisitorParam( node::NodeRequestsList ,3 ) const * _request ){};
	virtual void add( VisitorParam( node::NodeRequestsList ,4 ) const * _request ){};
	virtual void add( VisitorParam( node::NodeRequestsList ,5 ) const * _request ){};
	virtual void add( VisitorParam( node::NodeRequestsList ,6 ) const * _request ){};
	virtual void add( VisitorParam( node::NodeRequestsList ,7 ) const * _request ){};
	virtual void add( VisitorParam( node::NodeRequestsList ,8 ) const * _request ){};
	virtual void add( VisitorParam( node::NodeRequestsList ,9 ) const * _request ){};
	virtual void add( VisitorParam( node::NodeRequestsList ,10 ) const * _request ){};
	virtual ~CMedium(){};
};

template <>
class CMedium< monitor::MonitorResponses >
{
public:
	virtual bool serviced() const = 0;
	virtual bool flush() = 0;

	virtual bool getResponse( std::vector< monitor::MonitorResponses > & _requestResponse ) const = 0;
	virtual void clearResponses() = 0;
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,0 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,1 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,2 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,3 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,4 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,5 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,6 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,7 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorResponseList ,8 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,9 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,10 ) const * _request ){};
	virtual ~CMedium(){};
};

}

#endif // MEDIUM_H
