#ifndef CLIENT_RESPONSES_H
#define CLIENT_RESPONSES_H

#include <boost/statechart/event.hpp>
#include "protocol.h"

namespace client
{

struct CDnsInfo : boost::statechart::event< CDnsInfo >
{
	CDnsInfo( std::vector<CAddress> const & _addresses ):m_addresses( _addresses ){}
	vector<CAddress> m_addresses;
};

}

#endif // CLIENT_RESPONSES_H
