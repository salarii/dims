#ifndef TRACKER_RESPONSES_H
#define TRACKER_RESPONSES_H


#include "coins.h"
#include "nodeMessages.h"

#include <boost/variant.hpp>

namespace common
{

struct CDummy
{
	uint256 m_token;
	static CMainRequestType::Enum const  m_requestType;
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_token );
	)
};

struct CAvailableCoins
{
	CAvailableCoins( std::vector< CCoins > const & _availableCoins, uint256 const & _hash );
	CAvailableCoins();

	IMPLEMENT_SERIALIZE
	(
		int type;
		if ( !fRead )
			type = m_requestType;

		READWRITE(type);
		READWRITE(m_hash);
		READWRITE(m_availableCoins);
		READWRITE(m_availableCoins2);
	)

	static CMainRequestType::Enum const  m_requestType;
	uint256 m_hash;
	std::vector< CCoins > m_availableCoins;

	std::map< uint256, CCoins > m_availableCoins2;
};

typedef boost::variant< CDummy, CAvailableCoins > ClientResponse;

}

#endif // TRACKER_RESPONSES_H
