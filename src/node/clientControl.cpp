// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "clientControl.h"
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>


namespace client
{
CClientControl * CClientControl::ms_instance = 0;

struct CUninitiatedClient : boost::statechart::simple_state< CUninitiatedClient, CClientControl >
{
};


CClientControl*
CClientControl::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CClientControl();
	};
	return ms_instance;
}

CClientControl::CClientControl()
{
	initiate();
}

}
