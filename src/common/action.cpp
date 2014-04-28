// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "action.h"
#include "setResponseVisitor.h"

namespace common
{

inline
void
CAction::accept( CSetResponseVisitorBase & _visitor )
{
	_visitor.visit( *this );
}

}
