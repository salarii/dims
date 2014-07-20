// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "scanBitcoinNetworkRequest.h"
#include "common/mediumKinds.h"
#include "common/medium.h"

namespace tracker
{

CAskForTransactionsRequest::CAskForTransactionsRequest( std::vector< uint256 > const & _blockHashes, uint _mediumNumber )
	: m_blockHashes( _blockHashes )
	, m_mediumNumber( _mediumNumber )
{
}

void
CAskForTransactionsRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

int
CAskForTransactionsRequest::getKind() const
{
	return common::CMediumKinds::BitcoinsNodes;
}

std::vector< uint256 > const &
CAskForTransactionsRequest::getBlockHashes() const
{
	return m_blockHashes;
}

CSetBloomFilterRequest::CSetBloomFilterRequest( CBloomFilter const & _bloomFilter )
	: m_bloomFilter( _bloomFilter )
{
}

void
CSetBloomFilterRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

int
CSetBloomFilterRequest::getKind() const
{
	return common::CMediumKinds::BitcoinsNodes;
}

CBloomFilter const &
CSetBloomFilterRequest::getBloomFilter() const
{
	return m_bloomFilter;
}

}
