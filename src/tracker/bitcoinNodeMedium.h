#ifndef BITCOIN_NODE_MEDIUM_H
#define BITCOIN_NODE_MEDIUM_H

#include "configureTrackerActionHandler.h"
#include "common/medium.h"
#include "main.h"

class CNode;

namespace tracker
{

class CBitcoinNodeMedium : public common::CMedium< TrackerResponses >
{
public:
	CBitcoinNodeMedium( CNode * _node );

	bool serviced() const;

	bool flush();

	bool getResponseAndClear( std::vector< TrackerResponses > & _requestResponse );

	virtual void add( CAskForTransactionsRequest const * _request );

	virtual void add( CSetBloomFilterRequest const * _request );

	void setResponse( CTransaction const & _tx );

	void setResponse( CMerkleBlock const & _merkle );
private:
	void clearResponses();

	void reloadResponses();
private:
	mutable boost::mutex m_mutex;

	std::vector< TrackerResponses > m_responses;

	std::map< uint256 ,std::vector< CTransaction > > m_transactions;

	std::vector< CMerkleBlock > m_merkles;

	CNode * m_node;
};

}


#endif // BITCOIN_NODE_MEDIUM_H
