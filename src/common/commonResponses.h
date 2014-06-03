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
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_token );
	)
};

struct CAvailableCoins
{
	CAvailableCoins( std::map< uint256, CCoins > const & _availableCoins, uint256 const & _hash );
	CAvailableCoins();

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_hash);
		READWRITE(m_availableCoins);
	)

	static CMainRequestType::Enum const  m_requestType;
	uint256 m_hash;
	std::map< uint256, CCoins > m_availableCoins;
};


typedef boost::variant< CDummy, CAvailableCoins > ClientResponse;

}

#endif // TRACKER_RESPONSES_H
