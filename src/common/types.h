#ifndef TYPES_H
#define TYPES_H

#define MEDIUM_TYPE(_type) typename common::CGetMediumType<_type>::type
#define RESPONSE_TYPE(_type) typename common::CGetResponseType<_type>::type
#define FILTER_TYPE(_type) typename common::CGetFilterType<_type>::type

#include "tracker/configureTrackerActionHandler.h"
#include "client/configureClientActionHadler.h"
#include "monitor/configureMonitorActionHandler.h"
#include "seed/configureSeedActionHandler.h"

namespace common
{

		class CTrackerBaseMedium;
		struct CTrackerMediumFilter;

		class CMonitorBaseMedium;
		struct CMonitorMediumFilter;

		class CClientBaseMedium;
		struct CClientMediumFilter;

		class CSeedBaseMedium;
		struct CSeedMediumFilter;
}

namespace common
{

struct CTrackerTypes
{
	typedef tracker::TrackerResponses Response;
	typedef CTrackerBaseMedium Medium;
	typedef CTrackerMediumFilter Filter;
};

struct CMonitorTypes
{
	typedef monitor::MonitorResponses Response;
	typedef CMonitorBaseMedium Medium;
	typedef CMonitorMediumFilter Filter;
};

struct CClientTypes
{
	typedef client::ClientResponses Response;
	typedef CClientBaseMedium Medium;
	typedef CClientMediumFilter Filter;
};

struct CSeedTypes
{
	typedef seed::SeedResponses Response;
	typedef CSeedBaseMedium Medium;
	typedef CSeedMediumFilter Filter;
};

// get response
template < typename _Class >
struct CGetResponseType
{
	typedef int type;
};

template <>
struct CGetResponseType< CTrackerTypes >
{
	typedef CTrackerTypes::Response type;
};

template <>
struct CGetResponseType< CMonitorTypes >
{
	typedef CMonitorTypes::Response type;
};

template <>
struct CGetResponseType< CClientTypes >
{
	typedef CClientTypes::Response type;
};

template <>
struct CGetResponseType< CSeedTypes >
{
	typedef CSeedTypes::Response type;
};

template <>
struct CGetResponseType< CTrackerBaseMedium >
{
	typedef CTrackerTypes::Response type;
};

template <>
struct CGetResponseType< CClientBaseMedium >
{
	typedef CClientTypes::Response type;
};

template <>
struct CGetResponseType< CMonitorBaseMedium >
{
	typedef CMonitorTypes::Response type;
};

template <>
struct CGetResponseType< CSeedBaseMedium >
{
	typedef CSeedTypes::Response type;
};

//get medium
template < typename _Class >
struct CGetMediumType
{
	typedef int type;
};

template <>
struct CGetMediumType< CTrackerTypes >
{
	typedef CTrackerTypes::Medium type;
};

template <>
struct CGetMediumType< CMonitorTypes >
{
	typedef CMonitorTypes::Medium type;
};

template <>
struct CGetMediumType< CClientTypes >
{
	typedef CClientTypes::Medium type;
};

template <>
struct CGetMediumType< CSeedTypes >
{
	typedef CSeedTypes::Medium type;
};

template <>
struct CGetMediumType< CTrackerMediumFilter >
{
	typedef CTrackerTypes::Medium type;
};

template <>
struct CGetMediumType< CClientMediumFilter >
{
	typedef CClientTypes::Medium type;
};

template <>
struct CGetMediumType< CMonitorMediumFilter >
{
	typedef CMonitorTypes::Medium type;
};

template <>
struct CGetMediumType< CSeedMediumFilter >
{
	typedef CSeedTypes::Medium type;
};

// get filter
template < typename _Class >
struct CGetFilterType
{
	typedef int type;
};

template <>
struct CGetFilterType< CTrackerTypes >
{
	typedef CTrackerTypes::Filter type;
};

template <>
struct CGetFilterType< CMonitorTypes >
{
	typedef CMonitorTypes::Filter type;
};

template <>
struct CGetFilterType< CClientTypes >
{
	typedef CClientTypes::Filter type;
};

template <>
struct CGetFilterType< CSeedTypes >
{
	typedef CSeedTypes::Filter type;
};

}

#endif // TYPES_H
