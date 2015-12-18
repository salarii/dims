#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>

#include <string>
#include <algorithm>

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

std::vector<uint256>
substractVectors( std::vector<uint256> const & _a, std::vector<uint256> const & _b )
{
	std::vector<uint256> temp = _a;

	temp.erase( std::remove_if( temp.begin(), temp.end(), ExistsInVector( _b ) ), temp.end() );
	return temp;
}

std::vector<uint256> coins1 = boost::assign::list_of(1)(2)(3)(4)(5)(6)(7)(8)(9)(10)(11)(12)(13)(3)(4)(257)(258);

std::vector<uint256> coins1Check = boost::assign::list_of(1)(2)(3)(4)(5)(6)(7)(8)(9)(10)(11)(12)(13)(257)(258);

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
		tracker::CAddressToCoinsViewCache::getInstance()->setCoins(keyId_2, coin);
	}

	BOOST_FOREACH( uint256 const & coin, coins3 )
	{
		tracker::CAddressToCoinsViewCache::getInstance()->setCoins(keyId_3, coin);
	}

	tracker::CAddressToCoinsViewCache::getInstance()->flush();
}


BOOST_AUTO_TEST_CASE( basics )
{
	tracker::CAddressToCoinsViewCache::getInstance()->clearView();

	initialSetup();

	std::vector<uint256> checkCoins;

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_1, checkCoins );
	BOOST_CHECK( checkCoins == coins1Check );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_2, checkCoins );
	BOOST_CHECK( checkCoins == coins2 );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_3, checkCoins );
	BOOST_CHECK( checkCoins == coins3 );

	// test adding the same
	initialSetup();

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_1, checkCoins );
	BOOST_CHECK( checkCoins == coins1Check );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_2, checkCoins );
	BOOST_CHECK( checkCoins == coins2 );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_3, checkCoins );
	BOOST_CHECK( checkCoins == coins3 );

	tracker::CAddressToCoinsViewCache::getInstance()->clearView();

	BOOST_CHECK( !tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_1, checkCoins ) );

	BOOST_CHECK( !tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_2, checkCoins ) );

	BOOST_CHECK( !tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_3, checkCoins ) );
}

std::vector<uint256> remove1 = boost::assign::list_of(2)(3)(4)(5)(6)(257)(900);

std::vector<uint256> remove2 = std::vector<uint256>( coins1.begin()+1, coins1.begin() + 3 );

std::vector<uint256> remove3 = std::vector<uint256>( coins1.begin() + 2, coins1.begin() + 2 );


void
remove( uint160 const & _keyId, std::vector<uint256> const & _remove )
{
	BOOST_FOREACH( uint256 const & remove, _remove )
	{
		tracker::CAddressToCoinsViewCache::getInstance()->eraseCoins( _keyId, remove );
	}

	tracker::CAddressToCoinsViewCache::getInstance()->flush();
}

BOOST_AUTO_TEST_CASE( removal )
{
	tracker::CAddressToCoinsViewCache::getInstance()->clearView();

	initialSetup();

	remove( keyId_1, remove1 );
	remove( keyId_2, remove2 );
	remove( keyId_3, remove3 );

	std::vector<uint256> checkCoins;
	std::vector<uint256> expectedResult;
	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_1, checkCoins );

	expectedResult = substractVectors( coins1Check, remove1 );
	std::sort( checkCoins.begin(), checkCoins.end() );
	std::sort( expectedResult.begin(), expectedResult.end() );

	BOOST_CHECK( checkCoins == expectedResult );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_2, checkCoins );

	expectedResult = substractVectors( coins2, remove2 );
	std::sort( checkCoins.begin(), checkCoins.end() );
	std::sort( expectedResult.begin(), expectedResult.end() );

	BOOST_CHECK( checkCoins == expectedResult );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_3, checkCoins );

	expectedResult = substractVectors( coins3, remove3 );
	std::sort( checkCoins.begin(), checkCoins.end() );
	std::sort( expectedResult.begin(), expectedResult.end() );

	BOOST_CHECK( checkCoins == expectedResult );

	// remove twice

	remove( keyId_1, remove1 );
	remove( keyId_2, remove2 );
	remove( keyId_3, remove3 );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_1, checkCoins );

	expectedResult = substractVectors( coins1Check, remove1 );
	std::sort( checkCoins.begin(), checkCoins.end() );
	std::sort( expectedResult.begin(), expectedResult.end() );

	BOOST_CHECK( checkCoins == expectedResult );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_2, checkCoins );

	expectedResult = substractVectors( coins2, remove2 );
	std::sort( checkCoins.begin(), checkCoins.end() );
	std::sort( expectedResult.begin(), expectedResult.end() );

	BOOST_CHECK( checkCoins == expectedResult );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_3, checkCoins );

	expectedResult = substractVectors( coins3, remove3 );
	std::sort( checkCoins.begin(), checkCoins.end() );
	std::sort( expectedResult.begin(), expectedResult.end() );

	BOOST_CHECK( checkCoins == expectedResult );

	//restore
	initialSetup();

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_1, checkCoins );

	std::sort( checkCoins.begin(), checkCoins.end() );

	BOOST_CHECK( checkCoins == coins1Check );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_2, checkCoins );

	std::sort( checkCoins.begin(), checkCoins.end() );
	BOOST_CHECK( checkCoins == coins2 );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_3, checkCoins );

	std::sort( checkCoins.begin(), checkCoins.end() );
	BOOST_CHECK( checkCoins == coins3 );

	tracker::CAddressToCoinsViewCache::getInstance()->clearView();
}

// the  same  no  cache, until  first  get  cache is not used, test it
BOOST_AUTO_TEST_CASE( removalNocacheShort )
{
	tracker::CAddressToCoinsViewCache::getInstance()->clearView();

	initialSetup();

	remove( keyId_1, remove1 );
	remove( keyId_2, remove2 );
	remove( keyId_3, remove3 );

	// remove twice

	remove( keyId_1, remove1 );
	remove( keyId_2, remove2 );
	remove( keyId_3, remove3 );

	std::vector<uint256> checkCoins;
	std::vector<uint256> expectedResult;

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_1, checkCoins );

	expectedResult = substractVectors( coins1Check, remove1 );
	std::sort( checkCoins.begin(), checkCoins.end() );
	std::sort( expectedResult.begin(), expectedResult.end() );

	BOOST_CHECK( checkCoins == expectedResult );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_2, checkCoins );

	expectedResult = substractVectors( coins2, remove2 );
	std::sort( checkCoins.begin(), checkCoins.end() );
	std::sort( expectedResult.begin(), expectedResult.end() );

	BOOST_CHECK( checkCoins == expectedResult );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_3, checkCoins );

	expectedResult = substractVectors( coins3, remove3 );
	std::sort( checkCoins.begin(), checkCoins.end() );
	std::sort( expectedResult.begin(), expectedResult.end() );

	BOOST_CHECK( checkCoins == expectedResult );

	tracker::CAddressToCoinsViewCache::getInstance()->clearView();
}

BOOST_AUTO_TEST_CASE( removalNocacheLong )
{
	tracker::CAddressToCoinsViewCache::getInstance()->clearView();

	initialSetup();

	remove( keyId_1, remove1 );
	remove( keyId_2, remove2 );
	remove( keyId_3, remove3 );

	remove( keyId_1, remove1 );
	remove( keyId_2, remove2 );
	remove( keyId_3, remove3 );

	//restore
	initialSetup();

	std::vector<uint256> checkCoins;
	std::vector<uint256> expectedResult;

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_1, checkCoins );

	std::sort( checkCoins.begin(), checkCoins.end() );

	BOOST_CHECK( checkCoins == coins1Check );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_2, checkCoins );

	std::sort( checkCoins.begin(), checkCoins.end() );
	BOOST_CHECK( checkCoins == coins2 );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_3, checkCoins );

	std::sort( checkCoins.begin(), checkCoins.end() );
	BOOST_CHECK( checkCoins == coins3 );

	tracker::CAddressToCoinsViewCache::getInstance()->clearView();
}


BOOST_AUTO_TEST_CASE( regressionTest )
{
	tracker::CAddressToCoinsViewCache::getInstance()->clearView();
	tracker::CAddressToCoinsViewCache::getInstance()->setCoins( keyId_1, uint256(1) );
	tracker::CAddressToCoinsViewCache::getInstance()->setCoins( keyId_2, uint256(2) );
	tracker::CAddressToCoinsViewCache::getInstance()->setCoins( keyId_3, uint256(3) );
	tracker::CAddressToCoinsViewCache::getInstance()->flush();

	std::vector<uint256> checkCoins;
	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_3, checkCoins );
	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_2, checkCoins );
	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_1, checkCoins );
	BOOST_CHECK( checkCoins.front() == uint256(1) );

	tracker::CAddressToCoinsViewCache::getInstance()->eraseCoins( keyId_2, uint256(2) );

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_2, checkCoins );
	BOOST_CHECK( checkCoins.empty() );

	tracker::CAddressToCoinsViewCache::getInstance()->setCoins( keyId_1, uint256(257) );
	tracker::CAddressToCoinsViewCache::getInstance()->setCoins( keyId_2, uint256(1) );
	tracker::CAddressToCoinsViewCache::getInstance()->eraseCoins( keyId_1, uint256(110) );
	tracker::CAddressToCoinsViewCache::getInstance()->eraseCoins( keyId_1, uint256(1) );
	tracker::CAddressToCoinsViewCache::getInstance()->flush();

	tracker::CAddressToCoinsViewCache::getInstance()->getCoins( keyId_1, checkCoins );
	BOOST_CHECK( checkCoins.front() == uint256(257) );
}

BOOST_AUTO_TEST_SUITE_END()

