// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "requestHandler.h"
#include "common/nodeMessages.h"
#include "medium.h"
#include "sendInfoRequestAction.h"
#include "support.h"

#include "common/communicationBuffer.h"

using namespace common;

namespace node
{

CRequestHandler::CRequestHandler( CMedium * _medium )
	:m_usedMedium( _medium )
{
}

RequestResponse
CRequestHandler::getRespond( CRequest* _request ) const
{
	if( m_processedRequests.find( _request ) != m_processedRequests.end() )
		return m_processedRequests.find( _request )->second;
}

void
CRequestHandler::deleteRequest( CRequest* )
{

}

bool
CRequestHandler::isProcessed( CRequest* _request ) const
{
	if ( m_processedRequests.find( _request ) != m_processedRequests.end() )
		return true;
	
	return false;
}

bool 
CRequestHandler::setRequest( CRequest* _request )
{
	m_newRequest.push_back( _request );
}


void
CRequestHandler::runRequests()
{
	BOOST_FOREACH( CRequest* request, m_newRequest )
	{
		m_usedMedium->add( request );
	}

	m_usedMedium->flush();
}

void
CRequestHandler::readLoop()
{
    try
    {
        while(!m_usedMedium->serviced());

        common::CCommunicationBuffer response;
        m_usedMedium->getResponse(response);

        CBufferAsStream stream(
              (char*)response.m_buffer
            , response.m_usedSize
            , SER_DISK
            , CLIENT_VERSION);

        unsigned int counter = 0;
// this  logic  in bad, there is  no strict  relation between stream and request
        while( !stream.eof() )
        {
            int messageType;

            stream >> messageType;

			if ( messageType == CMainRequestType::ContinueReq )
            {
                uint256 token;
                stream >> token;
				m_processedRequests.insert( std::make_pair( m_newRequest[counter], CPending(token) ) );

            }
			else if ( messageType == CMainRequestType::TransactionStatusReq )
            {
                int status;
                stream >> status;
				//m_processedRequests.insert( std::make_pair( m_newRequest[counter], CTransactionStatus( (TransactionsStatus::Enum )status ) ) );

            }
			else if ( messageType == CMainRequestType::MonitorInfoReq )
            {

            }
			else if ( messageType == CMainRequestType::TrackerInfoReq )
            {
                 CTrackerStats trackerInfo;
                readTrackerInfo( stream, trackerInfo, TrackerDescription );
                m_processedRequests.insert( std::make_pair( m_newRequest[counter], trackerInfo ) );

            }
			else if ( messageType == CMainRequestType::RequestSatatusReq )
            {
            }
			else if ( messageType == CMainRequestType::BalanceInfoReq )
			{
				CAvailableCoins availableCoins;
				stream >> availableCoins;
				m_processedRequests.insert( std::make_pair( m_newRequest[counter], availableCoins ) );
			}
            else
            {
                throw;
            }
            counter++;
        }
        m_newRequest.clear();
    }
     catch (CMediumException & _mediumException)
    {
// maybe  here pass global errors  like  problems  with  network
// pass it here  but keep in mind that at least for now every single  action is responsible  for handling errors

        BOOST_FOREACH( CRequest* request, m_newRequest )
        {
            m_processedRequests.insert( std::make_pair( request, CSystemError( _mediumException.m_error ) ) );
        }
        m_newRequest.clear();
    }

}

}
