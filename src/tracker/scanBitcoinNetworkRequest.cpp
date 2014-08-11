// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "scanBitcoinNetworkRequest.h"
#include "common/mediumKinds.h"
#include "common/medium.h"
#include "trackerFilters.h"

namespace tracker
{

//common::CMediumKinds::BitcoinsNodes;
CAskForTransactionsRequest::CAskForTransactionsRequest( std::vector< uint256 > const & _blockHashes, common::CMediumFilter< TrackerResponses > * _mediumFilter )
	: common::CRequest< TrackerResponses >( _mediumFilter )
	, m_blockHashes( _blockHashes )
{
}

void
CAskForTransactionsRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< TrackerResponses > *
CAskForTransactionsRequest::getMediumFilter() const
{
	return m_mediumFilter;
}

std::vector< uint256 > const &
CAskForTransactionsRequest::getBlockHashes() const
{
	return m_blockHashes;
}

CSetBloomFilterRequest::CSetBloomFilterRequest( CBloomFilter const & _bloomFilter )
	: m_bloomFilter( _bloomFilter )
{
	m_mediumFilter = new CMediumClassFilter( common::CMediumKinds::BitcoinsNodes );
}

void
CSetBloomFilterRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< TrackerResponses > *
CSetBloomFilterRequest::getMediumFilter() const
{
	return m_mediumFilter;
}

CBloomFilter const &
CSetBloomFilterRequest::getBloomFilter() const
{
	return m_bloomFilter;
}

}
