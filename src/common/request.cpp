// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/filters.h"
#include "common/request.h"

#include "util.h"

namespace common
{

CRequest::CRequest( CMediumFilter * _mediumFilter )
	: m_mediumFilter( _mediumFilter )
	, m_processed( false )
{
	m_id = getRandNumber();
}

CRequest::CRequest( uint256 const & _id, CMediumFilter * _mediumFilter )
	: m_mediumFilter( _mediumFilter )
	, m_id( _id )
	, m_processed( false )
{
}

CMediumFilter *
CRequest::getMediumFilter() const
{ return m_mediumFilter; }

uint256
CRequest::getId() const
{
	return m_id;
}

CRequest::~CRequest()
{
	if ( m_mediumFilter )
		delete m_mediumFilter;
};

}
