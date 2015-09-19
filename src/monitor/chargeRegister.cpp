// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/analyseTransaction.h"
#include "common/authenticationProvider.h"

#include "monitor/chargeRegister.h"
#include "monitor/controller.h"

namespace monitor
{

CChargeRegister * CChargeRegister::ms_instance = NULL;

CChargeRegister*
CChargeRegister::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CChargeRegister();
	};
	return ms_instance;
}

void
CChargeRegister::addTransactionToSearch( uint256 const & _hash, CKeyID const & _keyId )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	m_searchTransaction.insert( std::make_pair( _hash, _keyId ) );
}

void
CChargeRegister::removeTransactionfromSearch( uint256 const & _hash )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_searchTransaction.erase( _hash );
}

void
CChargeRegister::loop()
{
	while(1)
	{
		{
			boost::lock_guard<boost::mutex> lock( m_mutex );

			std::map< uint256, CKeyID >::const_iterator iterator = m_searchTransaction.begin();

			std::list< uint256 > remove;
			while( iterator != m_searchTransaction.end() )
			{

				BOOST_FOREACH( CTransaction const & transaction, m_toSearch )
				{
					if ( iterator->first == transaction.GetHash() )
					{
						if ( common::findKeyInInputs( transaction, iterator->second ) )
						{
							std::vector < CTxOut > txOuts;
							std::vector< unsigned int > ids;

							common::findOutputInTransaction(
										transaction
										, common::CAuthenticationProvider::getInstance()->getMyKey().GetID()
										, txOuts
										, ids );

							unsigned int value = 0;
							BOOST_FOREACH( CTxOut const & txOut, txOuts )
							{
								value += txOut.nValue;
							}

							if ( CController::getInstance()->getPrice() <= value )
							{
								remove.push_back( iterator->first );
								m_acceptedTransactons.insert( iterator->first );
							}

						}

					}
				}

				iterator++;
			}

			BOOST_FOREACH( uint256 const & hash, remove )
			{
				m_searchTransaction.erase( hash );
			}
		}
		boost::this_thread::interruption_point();
		MilliSleep(500);
	}

}

bool
CChargeRegister::isTransactionPresent( uint256 const & _hash )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	return m_acceptedTransactons.find( _hash ) != m_acceptedTransactons.end();
}



}
