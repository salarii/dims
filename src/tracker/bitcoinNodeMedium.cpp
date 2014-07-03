// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bitcoinNodeMedium.h"

namespace tracker
{

CBitcoinNodeMedium::CBitcoinNodeMedium( CNode * _node )
	: m_node( _node )
{
}

bool
CBitcoinNodeMedium::serviced() const
{
	return true;
}

bool
CBitcoinNodeMedium::getResponse( std::vector< TrackerResponses > & _requestResponse ) const
{
	_requestResponse = m_responses;
	return true;
}

void
CBitcoinNodeMedium::clearResponses()
{
	m_responses.clear();
}


}
