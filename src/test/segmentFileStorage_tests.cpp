#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <string>

#include "util.h"
#include "version.h"
#include "core.h"
#include "tracker/segmentFileStorage.h"
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
		transaction.m_location = tracker::CSegmentFileStorage::getInstance()->assignPosition( _transaction );
		return transaction;
	}
};

BOOST_AUTO_TEST_CASE( basics )
{
	CBitcoinAddress  address( &GetNetworkParams< CChainParams >() );
	address.Set( CKeyID( uint160( Params().getOriginAddressKeyId() ) ) );
	std::string    addresso = address.ToString();

	tracker::CSegmentFileStorage * fileStorage = tracker::CSegmentFileStorage::getInstance();

	boost::thread( boost::bind(&tracker::CSegmentFileStorage::flushLoop, fileStorage) );

	std::vector< CTransaction > transactions = getTransactionArray();
	while(1)
	{
	std::transform( transactions.begin(), transactions.end(), transactions.begin(), CSetLocation() );

	fileStorage->includeTransactions( transactions, 0 );
	MilliSleep( 1000 );

	}

}

BOOST_AUTO_TEST_SUITE_END()
