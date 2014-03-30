
#include "informationProvider.h"
#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>


#include <boost/thread.hpp>


using boost::assign::map_list_of;
using namespace boost::assign;
using namespace boost;
using namespace monitorsScaner;

CInforamtionProvider::CInforamtionProvider()
{
// monitors
// ip  pub_key   name    price   enlisted_trackers
vec1 = list_of<std::list<std::string> >
(list_of("192.198.92.99:8333")("15sxZ93LFygg2Rvht6FSTtfxaTcGNSpSc5")("ala")("3456")("2"))
(list_of("46.137.222.237:8333")("1HMRX31xDbeuthaunjan9oeJtzsGLgutGG")("blo")("4456")("0"))
(list_of("188.138.9.208:8333")("16wzjmqzDg4qAQyKN2Wei17YJTJuSNNxs2")("udu")("56")("1"))
(list_of("88.198.240.138:8333")("1HPs3pbCcWnmozEauNKEETx9CmNRHs4ezQ")("aaa")("1")("3"))
(list_of("162.209.4.125:8333")("17GuvXRgi3NS5LuMmEBGF7ejzSg6mrzwo5")("bbb")("1000")("2"))
;

vec2 = list_of<std::list<std::string> >
(list_of("192.198.92.99:8333")("15sxZ93LFygg2Rvht6FSTtfxaTcGNSpSc5")("ala")("3456")("2"))
(list_of("46.137.222.237:8333")("1HMRX31xDbeuthaunjan9oeJtzsGLgutGG")("blo")("4456")("0"))
(list_of("188.138.9.208:8333")("16wzjmqzDg4qAQyKN2Wei17YJTJuSNNxs2")("fsd")("56")("1"))
(list_of("88.198.240.138:8333")("1HPs3pbCcWnmozEauNKEETx9CmNRHs4ezQ")("cbc")("1")("3"))
(list_of("205.186.129.90:46331")("18ytDTUgTChvgu4JRA2zTLFtiEocmzqAJy")("ppp")("100")("2"))
(list_of("198.199.109.12:8333")("1NCSLbBHF8CmZcwpWwvUMxnBR73FGirFWW")("nnnn")("188")("2"))
(list_of("188.40.112.72:8333")("12NepV7sEHb846Fic3gGhSRTCVcrhTBs1r")("vvvv")("90")("0"))
;


vec3 = list_of<std::list<std::string> >
		   (list_of("192.198.92.99:8333")("15sxZ93LFygg2Rvht6FSTtfxaTcGNSpSc5")("ala")("3456")("2"))
		   (list_of("46.137.222.237:8333")("1HMRX31xDbeuthaunjan9oeJtzsGLgutGG")("blo")("4456")("0"))
		   ;

vec4 = list_of<std::list<std::string> >
		   (list_of("192.198.92.99:8333")("15sxZ93LFygg2Rvht6FSTtfxaTcGNSpSc5")("ala")("3456")("2"))
		   (list_of("46.137.222.237:8333")("1HMRX31xDbeuthaunjan9oeJtzsGLgutGG")("blo")("4456")("0"))
		   (list_of("188.138.9.208:8333")("16wzjmqzDg4qAQyKN2Wei17YJTJuSNNxs2")("fsd")("56")("1"))
		   ;

// tracker
// ip pub_key  name   reputation  price
m_trackers = map_list_of<std::string,std::list<std::string> >
("15sxZ93LFygg2Rvht6FSTtfxaTcGNSpSc5", list_of("173.80.31.182:8333")("16jMDB9cnwhU6Voet5hDGqztoXqxpdpkZG")("aa")("1000")("0.5%"))
("15sxZ93LFygg2Rvht6FSTtfxaTcGNSpSc5", list_of("108.59.8.83:8333")("13xVwkFzwg1wpmxfgjBEbKN1qGH6K2cbZu")("bb")("200")("0.5%"))
("16wzjmqzDg4qAQyKN2Wei17YJTJuSNNxs2", list_of("173.168.25.229:8333")("1KpgGybuBchrZ8j8L5aLxgbro4NVvWTmMZ")("cc")("1100")("1%"))
("1HPs3pbCcWnmozEauNKEETx9CmNRHs4ezQ", list_of("46.183.217.193:8333")("12ZSJPNGRFTSycqACFkrRU7oMAEx6Tkst4")("dd")("1200")("1%"))
("1HPs3pbCcWnmozEauNKEETx9CmNRHs4ezQ", list_of("69.50.176.75:8333")("1Am5JYKHfF2cndp4uWjynHoat6xYUwaHoD")("ee")("1300")("0.2%"))
("1HPs3pbCcWnmozEauNKEETx9CmNRHs4ezQ", list_of("216.55.179.254:8312")("196kBVutJ9PPcZQ9rK5ksR2gqVyt9NU2Yp")("ff")("1400")("0.2%"))
("17GuvXRgi3NS5LuMmEBGF7ejzSg6mrzwo5", list_of("5.79.79.141:8333")("13scT7TusHFUsgSS3E4sbLnUJeWouWot3N")("gg")("1500")("0.2%"))
("17GuvXRgi3NS5LuMmEBGF7ejzSg6mrzwo5", list_of("54.227.255.14:8333")("1NVehoUBjCdr4DgVXsoHt1puQ9S78a8ypa")("hh")("1600")("1%"))
("18ytDTUgTChvgu4JRA2zTLFtiEocmzqAJy", list_of("198.58.107.157:8333")("1HC3dc4DubRat1P39YBBkwVRbph3ijbtPQ")("ii")("1700")("1%"))
("18ytDTUgTChvgu4JRA2zTLFtiEocmzqAJy", list_of("213.189.53.125:8333")("12HvVH9DKdyHTnDY5FwS3A1CUYBVfzUfPZ")("jj")("1800")("0.5%"))
("1NCSLbBHF8CmZcwpWwvUMxnBR73FGirFWW", list_of("94.23.213.14:8333")("16ibDU3zZdYp5MD25jxLjrunshKY3zwuhE")("kk")("1900")("0.5%"))
("1NCSLbBHF8CmZcwpWwvUMxnBR73FGirFWW", list_of("46.4.58.69:8337")("13LUcsR2bc4j6mmo7sdU9Y6v7oQqG67i1n")("mm")("2000")("0.5%"));


}

void
CInforamtionProvider::changeStorage()
{
	usedVectorTest = &vec3 == usedVectorTest ? &vec4 : &vec3;
	usedVectorMain = &vec1 ==  usedVectorTest ? &vec2 : &vec1;
}



void
CInforamtionProvider::getTrackers(monitorsScaner::Data& _trackers, std::string _publicKey ) const
{
	typedef std::multimap< std::string ,std::list<std::string> >::const_iterator TrackerInfoIterator;

	unsigned int rows = 0, columns = 0;
	 std::pair<  TrackerInfoIterator, TrackerInfoIterator> keyRange= m_trackers.equal_range(_publicKey);
	// Iterate over all map elements with key == theKey


	for (TrackerInfoIterator it = keyRange.first;  it != keyRange.second;  ++it)
	{
		rows++;
		columns = it->second.size();

		BOOST_FOREACH( std::string const & info, it->second )
		{
			_trackers.data.push_back( info );
		};
	}

	_trackers.cols = columns;
	_trackers.rows = rows;
}

typedef std::list<std::string> StringStorage;

void
CInforamtionProvider::getMonitorsInfo(monitorsScaner::Data& _monitors) const
{

	unsigned int rows = 0, columns = 0;
	BOOST_FOREACH( StringStorage const & storage, *usedVector )
	{
		rows++;
		columns = storage.size();
		BOOST_FOREACH( std::string const &data, storage )
		{
			_monitors.data.push_back( data );
		}
	}

	_monitors.cols = columns;
	_monitors.rows = rows;
}

void 
CInforamtionProvider::getInfo(monitorsScaner::Data& _return, const InfoRequest& infoRequest)
{
	if ( Info::TRACKERS_INFO )
	{	
		getTrackers(_return, infoRequest.key );
	}
	else if( Info::MONITORS_INFO )
	{
		if ( infoRequest.networkType == NetworkType::TESTNET )
		{
			usedVector = usedVectorTest;
		}
		else if ( infoRequest.networkType == NetworkType::MAIN )
		{
			usedVector = usedVectorMain;
		}
		else
		{
			Exception e;
			e.msg = "specify network type";
			throw e;
		}
		getMonitorsInfo(_return);
	}
	else 
	{
		Exception e;

		e.msg = "specify tracker or monitor info";
		throw e;
	}
}


void 
CInforamtionProvider::changeStorageThread( CInforamtionProvider & _informationProvider )
{
	while(1)
	{
		_informationProvider.changeStorage();
		boost::this_thread::sleep(boost::posix_time::seconds(180));
		boost::this_thread::interruption_point();
	}
}
