// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef	DUMMY_MEDIUM_H
#define DUMMY_MEDIUM_H

#include <list>

class CTrackersInfoRequest;

namespace node
{

class CDummyMedium
{
	struct CTrackerInfo
	{
		CTrackerInfo( std::string const & _ip, std::string const & _price, std::string const & _rating ):m_ip( _ip ),m_price( _price ),m_rating( _rating ){};
		std::string m_ip;
		std::string m_price;
		std::string m_rating;
	};

public:
	CDummyMedium();
	bool serviced() const;
	void add( CRequest const * _request );
	bool flush();
	bool getResponse( CCommunicationBuffer & _outBuffor ) const;
private:
	std::list< CTrackersInfoRequest * > m_trackerInfoRequests;
	bool m_serviced;

	CCommunicationBuffer m_buffer;

	CTrackerInfo const m_trackerInfo;
};


}

#endif // DUMMY_MEDIUM_H