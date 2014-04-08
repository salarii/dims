// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef SUPPORT_H
#define SUPPORT_H

namespace  node 
{

template < class T, class Enum >
void 
serializeEnum( T & _stream, Enum const _enum )
{
	int i = _enum;
	_stream << _enum;
};


}

#endif