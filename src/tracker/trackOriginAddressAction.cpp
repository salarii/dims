// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "trackOriginAddressAction.h"
#include "common/setResponseVisitor.h"
#include "common/mediumRequests.h"
#include "common/commonEvents.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "trackerMediumsKinds.h"
#include "main.h"
#include "chainparams.h"
#include "scanBitcoinNetworkRequest.h"

#define CONFIRM_LIMIT 6

namespace tracker
{


/*
store last  tracked  block  number
track blocks  between  last  checked  and present
use  at  least  three node  and  compare  results, in case of inconsistency of data  replace  wrong node  by  other correct  one


 what I need is bicoin node  medium

*/
struct CReadingData;


struct CUninitiated : boost::statechart::state< CUninitiated, CTrackOriginAddressAction >
{
	CUninitiated( my_context ctx ) : my_base( ctx )
	{
		context< CTrackOriginAddressAction >().setRequest( new common::CContinueReqest<TrackerResponses>( 0, CTrackerMediumsKinds::Internal ) );
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		if ( vNodes.size() >= 3 )
		{
			context< CTrackOriginAddressAction >().requestFiltered();// could proceed  with origin address scanning
			return transit< CReadingData >();
		}
		else
		{
			context< CTrackOriginAddressAction >().setRequest( new common::CContinueReqest<TrackerResponses>( 0, CTrackerMediumsKinds::Internal ) );
		}
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;

};

struct CReadingData : boost::statechart::state< CReadingData, CTrackOriginAddressAction >
{
	CReadingData( my_context ctx ) : my_base( ctx ), m_counter( 40 )
	{
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		if ( m_counter-- )
			context< CTrackOriginAddressAction >().setRequest( new common::CContinueReqest<TrackerResponses>( 0, CTrackerMediumsKinds::Internal ) );
		else
			return transit< CUninitiated >();
	}

	~CReadingData()
	{
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;

	uint m_counter;
};


CTrackOriginAddressAction::CTrackOriginAddressAction()
{
	initiate();

	CBlock block;

	if ( FileExist("head") )
	{
		CAutoFile file(OpenHeadFile(true), SER_DISK, CLIENT_VERSION);
		CBlockHeader header;
		file >> header;
		block = header;
	}
	else
	{
		block = const_cast<CBlock&>(Params().GenesisBlock());
	}

	m_currentHash = block.GetHash();

}



common::CRequest< TrackerResponses >*
CTrackOriginAddressAction::execute()
{
	return m_request;
}

void
CTrackOriginAddressAction::accept( common::CSetResponseVisitor< TrackerResponses > & _visitor )
{
	_visitor.visit( *this );
}

void
CTrackOriginAddressAction::setRequest( common::CRequest< TrackerResponses >* _request )
{
	m_request = _request;
}


// current  hash  distance  get  merkle  and  transaction
// came  back  to  the  same  state  over  and  over  till
//  analysys  will be finished


void
CTrackOriginAddressAction::requestFiltered()
{
	CBlockIndex * index = chainActive.Tip();
	// for  now  for  simplicity reasons
	for ( int i = 0; i < CONFIRM_LIMIT; i++ )
	{
		if ( index == 0 )
			break;
		index = index->pprev;
	}

	std::vector< uint256 > requestedBlocks;
	while ( m_currentHash != index->GetBlockHash() )
	{
		requestedBlocks.push_back( index->GetBlockHash() );

		index = index->pprev;
	}
	m_request = new CAskForTransactionsRequest( requestedBlocks );

}

// it should be  done  in fency style i  final version,
// but for  now I will keep it simple as much as possible

typedef std::map< long long, std::vector< CMerkleBlock > >::value_type MerkleResult;

typedef std::map< long long, std::map< uint256 , std::vector< CTransaction > > >::value_type TransactionsResult;


struct CCompareTransactions
{
	void verifyIfCorrect( std::vector< CTransaction > const & _transactions );

	bool isCorrect() const;

	std::vector< uint256 > m_hashes;

	bool m_correct;
};

void
CCompareTransactions::verifyIfCorrect( std::vector< CTransaction > const & _transactions )
{
	if ( !m_correct )
		return;

	std::vector< uint256 > hashes;

	BOOST_FOREACH( CTransaction const & transaction, _transactions )
	{
		hashes.push_back( transaction.GetHash() );
	}
	m_correct = m_hashes == hashes;
}

bool
CCompareTransactions::isCorrect() const
{
	return m_correct;
}


void
CTrackOriginAddressAction::analyseOutput( long long _key, std::vector< CMerkleBlock > const & _newInput )
{
	std::map< long long, std::vector< CMerkleBlock > >::iterator iterator = m_blocks.find( _key );

	std::vector< CMerkleBlock > accepted;

	if ( iterator == m_blocks.end() )
	{
		m_blocks.insert( std::make_pair( _key , _newInput ) );

		std::vector< CMerkleBlock > & merkle = m_blocks.find( _key )->second;

		validPart( merkle, accepted, merkle );
	}
	else
	{
		std::vector< CMerkleBlock > & merkle = iterator->second;

		merkle.insert( merkle.end(), _newInput.begin(), _newInput.end() );

		validPart( merkle, accepted, merkle );
	}

	iterator = m_acceptedBlocks.find( _key );

	if ( iterator == m_acceptedBlocks.end() )
	{
		m_blocks.insert( std::make_pair( _key , accepted ) );
	}
	else
	{
		iterator->second.insert( iterator->second.end(), accepted.begin(), accepted.end() );
	}

	// get size of  accepted

	uint size = -1;

	BOOST_FOREACH( MerkleResult & nodeResults, m_acceptedBlocks )
	{

		if ( size > nodeResults.second.size() )
			size = nodeResults.second.size();
	}
	// go  through transaction  queue analyse  if  the  same  content

	std::vector< uint256 > blocksToAccept;

	std::vector< CMerkleBlock > blockVector = m_acceptedBlocks.begin()->second;

	CBlockHeader headerToSave;

	for ( int i = 0; size > i; ++i )
	{
		blocksToAccept.push_back( blockVector.at( i ).header.GetHash() );

		if ( i == size -1 )
			headerToSave = blockVector.at( i ).header;

	}

	while( size-- )
	{
		std::vector<uint256> hashesVector, match;
		BOOST_FOREACH( MerkleResult & merkleVector, m_acceptedBlocks )
		{
			if ( hashesVector.empty() )
				if ( !merkleVector.second.at( size ).txn.ExtractMatches(hashesVector) )
					assert( !"shouldn't be here" );// there is problem
				else
				{
					if ( !merkleVector.second.at( size ).txn.ExtractMatches(match) )
						assert( !"shouldn't be here" );// there is problem
					if ( hashesVector != match )
						assert( !"shouldn't be here" );// problem which have to be resolved
				}
		}
		// compare  transactions

		BOOST_FOREACH( uint256 & hash, blocksToAccept )
		{
			CCompareTransactions compareTransactions;

			BOOST_FOREACH( TransactionsResult & transaction, m_transactions )
			{
				compareTransactions.verifyIfCorrect( transaction.second.find( hash )->second );
			}

			if ( !compareTransactions.isCorrect() )
				assert( !"data fro various nodes vary!" );// react  to  this

		}
	}
	// write  transactions to origin transaction scaner

	CAutoFile file(OpenHeadFile(false), SER_DISK, CLIENT_VERSION);
	file << headerToSave;

	fflush(file);

	FileCommit(file);

}

// return list of  hashes
// later return  list of  problems


void
CTrackOriginAddressAction::validPart( std::vector< CMerkleBlock > const & _input, std::vector< CMerkleBlock > & _accepted, std::vector< CMerkleBlock > & _rejected )
{
	std::vector< CMerkleBlock > output = _input;

	uint256 lastAcceptedHash = this->m_currentHash;

	CMerkleBlock & block = output.back();

	while ( output.empty() )
	{
		if ( block.header.hashPrevBlock == lastAcceptedHash )
		{
			lastAcceptedHash = block.header.hashPrevBlock;
			_accepted.push_back( block );
			output.pop_back();
		}
		else
		{
			std::vector< CMerkleBlock >::iterator iterator = output.begin();
			while( iterator != output.end() )
			{
				lastAcceptedHash = iterator->header.hashPrevBlock;

				output.erase( iterator );
				iterator++;
			}

			_rejected = output;
		}
	}

}


}
