// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VALIDATE_TRANSACTION_ACTION_EVENTS_H
#define VALIDATE_TRANSACTION_ACTION_EVENTS_H

#include <boost/statechart/event.hpp>

namespace tracker
{

struct CValidationEvent : boost::statechart::event< CValidationEvent >
{
	CValidationEvent( bool _valid ):m_valid( _valid ){}
	bool m_valid;
};


}


#endif // VALIDATE_TRANSACTION_ACTION_EVENTS_H
