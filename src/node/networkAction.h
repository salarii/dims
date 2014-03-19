#ifndef NETWORK_ACTION_H
#define NETWORK_ACTION_H

namespace node
{

struct ActionStatus
{
	enum Enum
	{
		 Unprepared
		,InProgress
	};
};



class CSendTransaction : public CAction
{
public:
	CSendTransaction( CTransaction & const _Transaction );

	void accept( CVisitor & _visitor );

	CRequest * execute();

	void setTransactionStatus( TransactionsStatus::Enum const _transactionStatus );

	void setTransactionToken( uint256 & const _token );
private:
	ActionStatus::Enum m_status;
	CTransaction m_transaction;
	TransactionsStatus::Enum m_status;
	uint256 m_token;
	
};


void
CSendTransaction::setTransactionStatus( TransactionsStatus::Enum const _transactionStatus )
{

}

void 
CSendTransaction::setTransactionToken( uint256 & const _token );

void
CSendTransaction::accept( CVisitor & _visitor )
{
	_visitor.visit( *this );
}


CRequest *
CSendTransaction::execute()
{
	if ( m_status == ActionStatus::Unprepared )
	{
		m_status = ActionStatus::InProgress;
		return new CTransactionSendRequest( m_transaction );
	}
	else
	{
		if ( m_status == TransactionsStatus::DoubleSpend );
			// double spend
		else if ( m_status == TransactionsStatus::Confirmed );
			//confirmed 
		else
			return new CTransactionSendRequest( m_token );
	}



	//create  transaction  message 
	// send transaction  message 

//  ping  for transaction 

// update  status  accordingly 

// maybe   visitor  needed  to  load  message  data 
}

struct CTransactionStatusRequest : public CRequest
{
public:
	CTransactionSendRequest( uint256 const & _token );
	void serialize( CBufferAsStream & _bufferStream );
	uint256 m_token;
};

CTransactionSendRequest::CTransactionSendRequest( uint256 const & _token )
	: m_token( _token )
{
}

CTransactionStatusRequest::serialize( CBufferAsStream & _bufferStream )
{
	_bufferStream << ( int ) CClientMessageType::Enum::TrackerInfoReq;
	_bufferStream << m_token;
}

struct CTransactionSendRequest : public CRequest
{
public:
	CTransactionSendRequest( CTransaction const & _transaction );
	void serialize( CBufferAsStream & _bufferStream );
	CTransaction m_transaction;
};

CTransactionSendRequest::CTransactionSendRequest( CTransaction const & _transaction )
	: m_transaction( _transaction )
{
}

void 
CTransactionSendRequest::serialize( CBufferAsStream & _bufferStream )
{
	_bufferStream << ( int ) CClientMessageType::Enum::Transaction;
	_bufferStream << m_transaction;
}

}