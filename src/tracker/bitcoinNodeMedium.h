#ifndef BITCOIN_NODE_MEDIUM_H
#define BITCOIN_NODE_MEDIUM_H

#include "configureTrackerActionHandler.h"
#include "common/medium.h"
#include "main.h"

class CNode;

namespace tracker
{
// current  implementation does not  pay  attention  to checking  if  responses are correctly assigned   to correspondent  request( I don't know if this may cause problems )
// besides this, it is primitie and ugly but  still may  serve
class CBitcoinNodeMedium : public common::CTrackerBaseMedium
{
public:
	CBitcoinNodeMedium( CNode * _node );

	bool serviced() const;

	bool flush();

	bool getResponseAndClear( std::multimap< common::CRequest< common::CTrackerTypes >const*, TrackerResponses, common::CLess< common::CRequest< common::CTrackerTypes > > > & _requestResponse );

	virtual void add( CAskForTransactionsRequest const * _request );

	virtual void add( CSetBloomFilterRequest const * _request );

	void setResponse( CTransaction const & _tx );

	void setResponse( CMerkleBlock const & _merkle );
private:
	void clearResponses();

	void reloadResponses();
private:
	mutable boost::mutex m_mutex;

	std::multimap< common::CRequest< common::CTrackerTypes >const*, TrackerResponses, common::CLess< common::CRequest< common::CTrackerTypes > > > m_responses;

	std::map< uint256 ,std::vector< CTransaction > > m_transactions;

	std::vector< CMerkleBlock > m_merkles;

	CNode * m_node;
};

}


#endif // BITCOIN_NODE_MEDIUM_H
