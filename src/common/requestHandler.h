// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "common/responses.h"

namespace common
{
/*
memory leak but  right  now  I can  live  with  that
?? now  I don't remember ..
*/
class CMedium;
class CRequest;
class CAction;

class CRequestHandler
{
public:
	CRequestHandler( CMedium * _medium );

	bool getLastResponse( CRequest * _request, common::DimsResponse & _response ) const;

	bool isProcessed( CRequest * _request ) const;

	void runRequests();

	void processMediumResponses();

	void deleteRequest( CRequest * );

	void clearLastResponse( CRequest * _request );

	bool operator==( CMedium const * _medium ) const;

	bool operator<( CMedium const * _medium ) const;

	void setRequest( CRequest * _request );

	bool operator<( CRequestHandler  const & _handler ) const;

	std::list< DimsResponse > getDirectActionResponse( CAction const * _action );// this is bad use but I don't know  what will really happen yet

	void deleteAction( CAction const * _action );
private:
	void setInvalid(){ m_valid = false; }
private:
	bool m_valid;

	std::vector<CRequest *> m_newRequest;

	std::multimap<CRequest const*,DimsResponse > m_processedRequests;

	CMedium * m_usedMedium;
};

}

#endif
