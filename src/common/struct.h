#ifndef STRUCT_H
#define STRUCT_H

#include "protocol.h"
#include "key.h"
#include "core.h"
namespace common
{

struct CValidNodeInfo
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_publicKey);
		READWRITE(m_address);
	)

	CValidNodeInfo()
	{
	}

	CValidNodeInfo( CPubKey const & _publicKey, CAddress const & _address ): m_publicKey( _publicKey ), m_address( _address )
	{
	}

	bool operator<( CValidNodeInfo const & _validNodeInfo ) const
	{
		return m_publicKey < _validNodeInfo.m_publicKey;
	}

	CPubKey m_publicKey;
	CAddress m_address;
};

struct CTrackerData : public CValidNodeInfo
{
	CTrackerData()
		: CValidNodeInfo()
		, m_reputation( 0 )
		, m_networkTime( 0 )
		, m_contractTime( 0 )
	{}

	CTrackerData( CPubKey _publicKey, CAddress const & _address, unsigned int _reputation, uint64_t _networkTime, uint64_t _contractTime )
		: CValidNodeInfo( _publicKey, _address )
		, m_reputation( _reputation )
		, m_networkTime( _networkTime )
		, m_contractTime( _contractTime )
	{}

	IMPLEMENT_SERIALIZE
	(
		READWRITE((CValidNodeInfo &)*this);
		READWRITE(m_reputation);
		READWRITE(m_networkTime);
		READWRITE(m_contractTime);
	)

	unsigned int m_reputation;
	uint64_t m_networkTime;
	uint64_t m_contractTime;

	bool operator<( CTrackerData const & _trackerData ) const
	{
		return m_publicKey < _trackerData.m_publicKey;
	}
};

//
struct CAllyMonitorData : public CValidNodeInfo
{
	CAllyMonitorData(){}
	CAllyMonitorData( CPubKey const &_publicKey, CAddress const & _address )
		: CValidNodeInfo( _publicKey, _address ){}

	IMPLEMENT_SERIALIZE
	(
		READWRITE((CValidNodeInfo &)*this);
	)
};

struct CAllyTrackerData : public CTrackerData
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(*this);
		READWRITE(m_allyMonitorKey);
	)

	CAllyTrackerData(){}

	CAllyTrackerData( CTrackerData const & _trackerData, CPubKey const & _monitorKey )
		: CTrackerData( _trackerData )
		, m_allyMonitorKey( _monitorKey )
	{}

	CPubKey m_allyMonitorKey;
};

struct CUnidentifiedNodeInfo
{
	IMPLEMENT_SERIALIZE
	(
	READWRITE(m_ip);
	READWRITE(m_port);
	)
	CUnidentifiedNodeInfo(	std::string const & _ip, unsigned int _port ):m_ip( _ip ), m_port( _port ){}
	std::string m_ip;
	unsigned int m_port;

	bool operator<( CUnidentifiedNodeInfo const & _unidentifiedStats ) const
	{
		return m_ip < _unidentifiedStats.m_ip;
	}
};

struct CNodeInfo : public CUnidentifiedNodeInfo
{
	IMPLEMENT_SERIALIZE
	(
	READWRITE(*(CUnidentifiedNodeInfo*)this);
	READWRITE(m_key);
	READWRITE(m_role);
	)

	CNodeInfo( CPubKey const & _key = CPubKey(), std::string _ip = std::string(), unsigned int _port = 0, unsigned int _role = -1 ): CUnidentifiedNodeInfo( _ip, _port ), m_key( _key ), m_role( _role ){}
	CPubKey m_key;
	unsigned int m_role;
};

struct CMonitorData
{
	// most likely self public key, should goes here
	IMPLEMENT_SERIALIZE
	(
			READWRITE( m_trackers );
			READWRITE( m_monitors );
			READWRITE( m_signed );
	)

	CMonitorData(){};

	CMonitorData( std::set< common::CTrackerData > const & _trackers, std::vector< common::CNodeInfo > const & _monitors ):m_trackers( _trackers ), m_monitors( _monitors ){};

	std::set< common::CTrackerData > m_trackers;
	std::vector< common::CNodeInfo > m_monitors;
	std::vector<unsigned char> m_signed;
	// recognized  monitors and trackers
};

struct CPayApplicationData
{
	CPayApplicationData(){}

	CPayApplicationData(
			CTransaction const & _trasaction
			, std::vector<unsigned char> const & _transactionStatusSignature
			, CPubKey const & _servicingTracker
			, common::CMonitorData const & _monitorData
			, CPubKey const & _servicingMonitor
			)
		: m_trasaction( _trasaction )
		, m_transactionStatusSignature( _transactionStatusSignature )
		, m_servicingTracker( _servicingTracker )
		, m_monitorData( _monitorData )
		, m_servicingMonitor( _servicingMonitor )
	{}
	IMPLEMENT_SERIALIZE
	(
	READWRITE(m_trasaction);
	READWRITE(m_transactionStatusSignature);
	READWRITE(m_servicingTracker);
	READWRITE(m_monitorData);
	READWRITE(m_servicingMonitor);
	)
	CTransaction m_trasaction;
	std::vector<unsigned char> m_transactionStatusSignature;
	CPubKey m_servicingTracker;
	common::CMonitorData m_monitorData;
	CPubKey m_servicingMonitor;
};

}

#endif // STRUCT_H
