// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACK_ORIGIN_ADDRESS_ACTION_H
#define TRACK_ORIGIN_ADDRESS_ACTION_H

#include "common/action.h"
#include "configureTrackerActionHandler.h"
#include <boost/statechart/state_machine.hpp>
#include "main.h"

// please add excluding of bad nodes( not responding )
namespace tracker
{
struct CUninitiatedTrackAction;

class CTrackOriginAddressAction : public common::CAction< TrackerResponses >, public  boost::statechart::state_machine< CTrackOriginAddressAction, CUninitiatedTrackAction >
{
public:
	CTrackOriginAddressAction();

	virtual common::CRequest< TrackerResponses >* execute();

	virtual void accept( common::CSetResponseVisitor< TrackerResponses > & _visitor );

	void setRequest( common::CRequest< TrackerResponses >* _request );

	void requestFiltered();

	void analyseOutput( long long _key, std::map< uint256 ,std::vector< CTransaction > > const & _newTransactions, std::vector< CMerkleBlock > const & _newInput );

	void clear();

	void clearAccepted( uint const _number );
private:
	void validPart( long long _key, std::vector< CMerkleBlock > const & _input, std::vector< CMerkleBlock > & _rejected );
private:
	//replace std::vector with std::deque ???
	std::map< long long, std::vector< CMerkleBlock > > m_blocks;
	//replace std::vector with std::deque ???
	std::map< long long, std::vector< CMerkleBlock > > m_acceptedBlocks;

	std::map< long long, std::map< uint256 , std::vector< CTransaction > > > m_transactions;

	common::CRequest< TrackerResponses >* m_request;

	uint256 m_currentHash;
};

}

#endif // TRACK_ORIGIN_ADDRESS_ACTION_H
