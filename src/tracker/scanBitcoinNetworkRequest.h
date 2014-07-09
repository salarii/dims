// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SCAN_BITCOIN_NETWORK_REQUEST_H
#define SCAN_BITCOIN_NETWORK_REQUEST_H

#include "common/request.h"

#include "configureTrackerActionHandler.h"

#include "bloom.h"

namespace tracker
{

class CAskForTransactionsRequest : public common::CRequest< TrackerResponses >
{
public:
	CAskForTransactionsRequest( std::vector< uint256 > const & _blockHashes, uint _mediumNumber );

	virtual void accept( common::CMedium< TrackerResponses > * m_mediumNumber ) const;

	virtual int getKind() const;

	virtual unsigned int getMediumNumber() const { return 3; }

	std::vector< uint256 > const & getBlockHashes() const;
private:
	std::vector< uint256 > const m_blockHashes;

	uint m_mediumNumber;
};


class CSetBloomFilterRequest : public common::CRequest< TrackerResponses >
{
public:
	CSetBloomFilterRequest( CBloomFilter const & _bloomFilter );

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual int getKind() const;

	CBloomFilter const & getBloomFilter() const;
private:
	CBloomFilter const m_bloomFilter;
};

}


#endif // SCAN_BITCOIN_NETWORK_REQUEST_H
