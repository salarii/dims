#ifndef TRACKER_RESPONSES_H
#define TRACKER_RESPONSES_H


#include "coins.h"
#include "nodeMessages.h"

#include <boost/variant.hpp>

#include <exception>
#include "errorResponse.h"

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

struct CIdentificationResult
{
	CIdentificationResult( std::vector<unsigned char> const & _payload, std::vector<unsigned char> const & _signed, uint160 const & _key ):m_payload( _payload ),m_signed( _signed ),m_key( _key ){};
	std::vector<unsigned char> m_payload;
	std::vector<unsigned char> m_signed;
	CKeyID m_key;
};


struct CContinueResult
{
	CContinueResult( uint256 const &_key ):m_id(_key){};
	uint256 m_id;
};

struct CMediumException : public std::exception
{
public:
	CMediumException(ErrorType::Enum _error):m_error(_error){};
	ErrorType::Enum m_error;
};

typedef boost::variant< CDummy, CAvailableCoins > ClientResponse;

}

#endif // TRACKER_RESPONSES_H
