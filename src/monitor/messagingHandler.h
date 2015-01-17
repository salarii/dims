// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MESSAGING_HANDLER_H
#define MESSAGING_HANDLER_H

namespace monitor
{

class CMessagingHandler
{
public:
	template < class T >
	void sendMessage( T _message );

	void receivedMessageHandler();
private:
};

template < class _T >
void
CMessagingHandler::sendMessage()
{

}

}

#endif


