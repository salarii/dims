// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_NODE_MEDIUM_H
#define BITCOIN_NODE_MEDIUM_H

#include "common/medium.h"
#include "common/requests.h"

#include "bitcoinNodeMedium.h"
#include "chainparams.h"
#include "main.h"

class CNode;

namespace common
{

class CAskForTransactionsRequest;

class CSetBloomFilterRequest;

// current  implementation does not  pay  attention  to checking  if  responses are correctly assigned   to correspondent  request( I don't know if this may cause problems )
// besides this, it is primitie and ugly but  still may  serve
class CBitcoinNodeMedium : public CMedium
{
public:
	CBitcoinNodeMedium( CNode * _node );

	bool serviced() const;

	bool flush();

	bool getResponseAndClear( std::multimap< common::CRequest const*, DimsResponse > & _requestResponse );

	virtual void add( CAskForTransactionsRequest  const * _request );

	virtual void add( CSetBloomFilterRequest  const * _request );

	void setResponse( CTransaction const & _tx );

	void setResponse( CMerkleBlock const & _merkle );

	~CBitcoinNodeMedium();
private:
	void clearResponses();

	void reloadResponses();
private:
	mutable boost::mutex m_mutex;

	std::multimap< common::CRequest const*, DimsResponse > m_responses;

	std::map< uint256 ,std::vector< CTransaction > > m_transactions;

	std::vector< CMerkleBlock > m_merkles;

	CNode * m_node;

	common::CRequest * LastRequest;//ugly way  to do responses <-> request  matching
};

}

#endif // BITCOIN_NODE_MEDIUM_H
