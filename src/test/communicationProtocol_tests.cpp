#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <string>

#include "util.h"
#include "version.h"
#include "core.h"
#include "tracker/communicationProtocol.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_reader_template.h"
#include "data/tx_valid.json.h"

using namespace std;
using namespace json_spirit;

extern Array
read_json(const std::string& jsondata);

BOOST_AUTO_TEST_SUITE(communicationProtocol_tests)



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
}

BOOST_AUTO_TEST_CASE( basics )
{
	tracker::CMessage( getTransactionArray() );
	//BOOST_CHECK(uint160(std::vector<unsigned char>(OneArray,OneArray+19)) == 0);
}


BOOST_AUTO_TEST_SUITE_END()
