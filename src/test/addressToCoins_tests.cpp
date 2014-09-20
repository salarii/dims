#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>
#include <string>

#include "util.h"
#include "version.h"
#include "core.h"

#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_reader_template.h"
#include "data/tx_valid.json.h"
#include "base58.h"

#include "tracker/addressToCoins.h"

using namespace std;
using namespace json_spirit;


BOOST_AUTO_TEST_SUITE(addressToCoins_tests)


/*

struct ExistsInVector
{
	ExistsInVector(const std::vector<uint256>& vec) : m_vec(vec) {
	}
	bool operator() (uint256 i) {
		return (std::find(m_vec.begin(), m_vec.end(), i) != m_vec.end());
	}
private:
	const std::vector<uint256>& m_vec;
};


std::remove_if(A.begin(), A.end(), ExistsInVector(B));

A.erase( std::remove_if(A.begin(), A.end(), ExistsInVector(B)), A.end() );


*/

std::vector<uint256> coins1 = boost::assign::list_of(1)(2)(3)(4)(6)(7)(8)(9)(10)(11)(12)(13);

std::vector<uint256> coins2 = std::vector<uint256>( coins1.begin(), coins1.begin() + 5 );

std::vector<uint256> coins3 = std::vector<uint256>( coins1.begin() + 2, coins1.begin() + 7 );

uint160 keyId_1(1);
uint160 keyId_2(2);
uint160 keyId_3(3);



void initialSetup()
{
	BOOST_FOREACH( uint256 const & coin, coins1 )
	{
		tracker::CAddressToCoinsViewCache::getInstance()->setCoins(keyId_1, coin);
	}

	BOOST_FOREACH( uint256 const & coin, coins2 )
	{
		tracker::CAddressToCoinsViewCache::getInstance()->setCoins(keyId_1, coin);
	}

	BOOST_FOREACH( uint256 const & coin, coins3 )
	{
		tracker::CAddressToCoinsViewCache::getInstance()->setCoins(keyId_1, coin);
	}

	tracker::CAddressToCoinsViewCache::getInstance()->flush();
}


BOOST_AUTO_TEST_CASE( basics )
{
	tracker::CAddressToCoinsViewCache::getInstance()->clearView();

	initialSetup();

	std::vector<uint256> checkCoins;

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_1, checkCoins );
	BOOST_CHECK( checkCoins == coins1 );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_2, checkCoins );
	BOOST_CHECK( checkCoins == coins2 );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_3, checkCoins );
	BOOST_CHECK( checkCoins == coins3 );

	tracker::CAddressToCoinsViewCache::getInstance()->clearView();

	BOOST_CHECK( !tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_1, checkCoins ) );

	BOOST_CHECK( !tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_2, checkCoins ) );

	BOOST_CHECK( !tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_3, checkCoins ) );
}

BOOST_AUTO_TEST_SUITE_END()

