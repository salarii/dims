#ifndef TRACKER_RESPONSES_H
#define TRACKER_RESPONSES_H

#include "node/support.h"

#include "coins.h"
#include "nodeMessages.h"

#include <boost/variant.hpp>

namespace tracker
{

struct CDummy
{
	static CMainRequestType::Enum const  m_requestType;

	template < class Stream >
	void Serialize(Stream &s, int nType, int nVersion) const
	{
		node::serializeEnum( s, m_requestType );
	}
};

struct CAvailableCoins
{
	CAvailableCoins( std::vector< CCoins > const & _availableCoins, uint256 const & _hash );

	template < class Stream >
	void Serialize(Stream &s, int nType, int nVersion) const
	{
		node::serializeEnum( s, m_requestType );
		s << m_hash;
		s << m_availableCoins;
	}

	static CMainRequestType::Enum const  m_requestType;
	uint256 m_hash;
	std::vector< CCoins > m_availableCoins;
};

typedef boost::variant< CDummy, CAvailableCoins > ClientResponse;



}

#endif // TRACKER_RESPONSES_H
