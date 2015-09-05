#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <string>

#include "util.h"
#include "version.h"
#include "core.h"

#include "common/segmentFileStorage.h"

#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_reader_template.h"
#include "data/tx_valid.json.h"
#include "base58.h"
using namespace std;
using namespace json_spirit;

Array
read_json(const std::string& jsondata)
{
	Value v;

	if (!read_string(jsondata, v) || v.type() != array_type)
	{
		BOOST_ERROR("Parse error.");
		return Array();
	}
	return v.get_array();
}

BOOST_AUTO_TEST_SUITE(segmentFileStorage_tests)

// ... where all scripts are stringified scripts.

std::vector< CTransaction >
getTransactionArray()
{
	Array tests = read_json(std::string(json_tests::tx_valid, json_tests::tx_valid + sizeof(json_tests::tx_valid)));

	std::vector< CTransaction > transactions;

	BOOST_FOREACH(Value& tv, tests)
	{
		Array test = tv.get_array();
		string strTest = write_string(tv, false);
		if (test[0].type() == array_type)
		{
			if (test.size() != 3 || test[1].type() != str_type || test[2].type() != bool_type)
			{
				BOOST_ERROR("Bad test: " << strTest);
				continue;
			}


			string transaction = test[1].get_str();
			CDataStream stream(ParseHex(transaction), SER_NETWORK, PROTOCOL_VERSION);
			CTransaction tx;
			stream >> tx;
			
			transactions.push_back( tx );
		}
	}
	return transactions;
}
/*
 : binary_function <T,T,T> {
  T operator() (const T& x, const T& y) const {return x+y;}
};
 */
struct CSetLocation
{
	CTransaction operator()( CTransaction const & _transaction )
	{
		CTransaction transaction( _transaction );
		transaction.m_location = common::CSegmentFileStorage::getInstance()->assignPosition( _transaction );
		return transaction;
	}
};

BOOST_AUTO_TEST_CASE( basics )
{

	common::CSegmentFileStorage * fileStorage = common::CSegmentFileStorage::getInstance();
// investigate  bug


	common::CDimsParams::setAppType( common::AppType::Tracker);

	common::CSegmentHeader * segmentHeader = new common::CSegmentHeader;
	//common::CSegmentFileStorage::getInstance()->getCopySegmentHeader( 0, *segmentHeader );

	//common::CSegmentFileStorage::getInstance()->setDiscBlock( *segmentHeader, 0 );


	common::CDiskBlock * block =  new common::CDiskBlock;

fileStorage->getCopyBlock( 0, *block );
std::vector< CTransaction >transactions;

common::CDiskBlock * block1 =  new common::CDiskBlock;

int  level = fileStorage->getLevel( 3074 );
block->buddyFree(fileStorage->getIndex( 3074 ));
int k;
k = block1->buddyAlloc( 12 );
k = block1->buddyAlloc( 12 );
k = block1->buddyAlloc( 11 );
block1->buddyFree(1);
block1->buddyFree(0);
block1->buddyFree(2);
fileStorage->readTransactions( *block, transactions );

	uint64_t position = fileStorage->assignPosition( transactions.front() );


CTransaction transaction = transactions.front();




{
	int index = block1->buddyAlloc( 10 );
	CBufferAsStream stream(
				(char *)block1->translateToAddress( fileStorage->getIndex( position ) )
				, block1->getBuddySize( fileStorage->getLevel( position ) )
				, SER_DISK
				, CLIENT_VERSION);
	stream << transaction;
}
fileStorage->readTransactions( *block1, transactions );
fileStorage->saveBlock( 1, *block1 );
fileStorage->getCopyBlock( 1, *block );
fileStorage->saveBlock( 0, *(new common::CDiskBlock() ) );
fileStorage->getCopyBlock( 1, *block );
transactions.front();
/*
	boost::thread( boost::bind(&common::CSegmentFileStorage::flushLoop, fileStorage) );

	std::vector< CTransaction > transactions = getTransactionArray();
	while(1)
	{
	std::transform( transactions.begin(), transactions.end(), transactions.begin(), CSetLocation() );



	// right now  it  will not  work, because transactions have  to  rely on  each  other
	//	m_supportTransactionsDatabase->setTransactionLocation( _tx.GetHash(), tx.m_location );
	fileStorage->includeTransactions( transactions, 0 );

	MilliSleep( 1000 );

	}
*/
}

BOOST_AUTO_TEST_SUITE_END()
