// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CUSTODY_H
#define CUSTODY_H

namespace monitor
{

class CCustody
{
public:
	CCustody();
	void loop();

	void addToCustody( uint256 _trackerHash );
private:
	int64_t getSentanceTime();

	void announceSentenceEnd( uint256 _trackerHash );
private:
	std::list< uint256 > m_detained;
};

CCustody::CCustody()
{
}

void 
CCustody::loop()
{
	while(1)
	{
		std::list< uint256 >::iterator iterator = m_detained.begin();

		while( iterator != m_detained.end() )
		{
			if ( getSentanceTime( *iterator ) < GetTime() )
			{
				announceSentenceEnd( *iterator );
				iterator++;
				m_detained.erase( iterator - 1 );
			}
		}

		sleep( 1 minute);
	}
}

}

#endif


