#ifndef BITCOIN_NODE_MEDIUM_H
#define BITCOIN_NODE_MEDIUM_H

#include "configureTrackerActionHandler.h"
#include "common/medium.h"

class CNode;

namespace tracker
{

class CBitcoinNodeMedium : public common::CMedium< TrackerResponses >
{
public:
	CBitcoinNodeMedium( CNode * _node );

	bool serviced() const;

	bool flush();

	bool getResponse( std::vector< TrackerResponses > & _requestResponse ) const;

	void clearResponses();
private:
	mutable boost::mutex m_mutex;

	std::vector< TrackerResponses > m_responses;

	CNode * m_node;
};

}


#endif // BITCOIN_NODE_MEDIUM_H
