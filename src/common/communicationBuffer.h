// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define MaxBufferSize ( 1 << 14 )

namespace common
{

struct CCommunicationBuffer
{
	CCommunicationBuffer():m_usedSize(0){}
	char m_buffer[ MaxBufferSize ];
	unsigned int m_usedSize;
};
}

