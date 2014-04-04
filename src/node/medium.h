// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef	MEDIUM_H
#define MEDIUM_H

#define MaxBufferSize 1000

namespace node
{

struct CCommunicationBuffer;
struct CRequest;

struct CCommunicationBuffer
{
	char m_buffer[ MaxBufferSize ];
	unsigned int m_usedSize;
};

class CMedium
{
public:
	virtual bool serviced() const = 0;
	virtual void add( CRequest const * _request ) = 0;
	virtual bool flush() = 0;
	virtual bool getResponse( CCommunicationBuffer & _outBuffor ) const = 0;
};


}

#endif // MEDIUM_H