#ifndef COMMON_STRUCT_H
#define COMMON_STRUCT_H

#include "protocol.h"
#include "key.h"

namespace common
{

struct CValidNodeInfo
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_key);
		READWRITE(m_address);
	)

	CValidNodeInfo()
	{
	}

	CValidNodeInfo( CPubKey _key, CAddress _address ):m_key( _key ), m_address( _address )
	{
	}

	bool operator<( CValidNodeInfo const & _validNodeInfo ) const
	{
		return m_key < _validNodeInfo.m_key;
	}

	CPubKey m_key;
	CAddress m_address;
};

struct CTrackerData
{
	CTrackerData()
		: m_publicKey( CPubKey() )
		, m_reputation( 0 )
		, m_networkTime( 0 )
		, m_contractTime( 0 )
	{}

	CTrackerData( CPubKey _publicKey, unsigned int _reputation, uint64_t _networkTime, uint64_t _contractTime ): m_publicKey( _publicKey ),m_reputation( _reputation ), m_networkTime( _networkTime ), m_contractTime( _contractTime ){}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_publicKey);
		READWRITE(m_reputation);
		READWRITE(m_networkTime);
		READWRITE(m_contractTime);
	)

	CPubKey m_publicKey;
	unsigned int m_reputation;
	uint64_t m_networkTime;
	uint64_t m_contractTime;
};


struct CAllyMonitorData : public common::CValidNodeInfo
{
	CAllyMonitorData(){}
	CAllyMonitorData( CPubKey const &_publicKey ): m_publicKey( _publicKey ){}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_publicKey);
	)

	CPubKey m_publicKey;
};

struct CAllyTrackerData
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_trackerKey);
		READWRITE(m_allyMonitorKey);
		READWRITE(m_reputation);
		READWRITE(m_networkTime);
		READWRITE(m_previousNetworkTime);
		READWRITE(m_countedTime);
	)

	CPubKey m_trackerKey;
	CPubKey m_allyMonitorKey;
	unsigned int m_reputation;
	uint64_t m_networkTime;
	uint64_t m_previousNetworkTime;
	unsigned int m_countedTime;
};

}

#endif // COMMON_STRUCT_H
