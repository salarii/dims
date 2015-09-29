// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef STALLING_H
#define STALLING_H

#include <openssl/sha.h>
#include <vector>
#include <algorithm> 

namespace tracker
{

bool condition( unsigned int _leadinZeroNumber,unsigned char const (&_hash)[SHA256_DIGEST_LENGTH])
{
	for ( unsigned int i = SHA256_DIGEST_LENGTH - 1; i >= 0; --i )
	{
		if ( _leadinZeroNumber >= 8 )
		{
			if ( _hash[ i ] != 0 )
				return false;
			_leadinZeroNumber -= 8;
		}
		else
		{
			unsigned char compare = 0xff;
			compare >>= _leadinZeroNumber;
			if ( compare < _hash[ i ] )
				return false;
			else
				return true;
		}
	}
	return true;

}


bool
proofOfWorkChecking(std::vector< unsigned char > & _payload, unsigned int _leadinZeroNumber )
{
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, &_payload[0], _payload.size());
	SHA256_Final(hash, &sha256);

	return condition( _leadinZeroNumber, hash);
}


struct CIncreaseCell
{
	static bool m_increase;
	CIncreaseCell(){}
	unsigned char operator() ( unsigned char & _cell ) const
	{
		if ( m_increase )
		{
			_cell = ++_cell % 0x100;

			m_increase = !_cell;
		}
		return _cell;
	}
};

bool CIncreaseCell::m_increase = false;



void
sha256(std::vector< unsigned char > & _payload, unsigned int _leadinZeroNumber )
{
	unsigned char hash[SHA256_DIGEST_LENGTH];

	do
	{	
		CIncreaseCell::m_increase = true;
		std::transform (_payload.rbegin(), _payload.rend(), _payload.rbegin(), CIncreaseCell());
		if ( CIncreaseCell::m_increase )
			_payload.push_back( 0 );

		SHA256_CTX sha256;
		SHA256_Init(&sha256);
		SHA256_Update(&sha256, &_payload[0], _payload.size());
		SHA256_Final(hash, &sha256);

	}
	while( !condition( _leadinZeroNumber, hash) );

}




}
