// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "sendTransactionAction.h"
#include "common/requestResponse.h"
#include "connectAction.h"
#include "sendBalanceInfoAction.h"
#include "common/responseVisitorInternal.h"
#include "configureNodeActionHadler.h"

#include <boost/any.hpp>
#include <boost/optional.hpp>

namespace common
{

template < class _Action >
class CGetTransactionStatus : public CResponseVisitorBase< _Action, node::NodeResponseList >
{
public:
	CGetTransactionStatus( _Action * const _action ):CResponseVisitorBase< _Action, node::NodeResponseList >( _action ){};

	void operator()(CTransactionStatus & _transactionStatus ) const
	{
		this->m_action->setTransactionStatus( _transactionStatus.m_status );
	}

};

template < class _Action >
class CGetToken : public CResponseVisitorBase< _Action, node::NodeResponseList >
{
public:
	CGetToken( _Action * const _action ):CResponseVisitorBase< _Action, node::NodeResponseList >( _action ){};

	void operator()(CTransactionStatus & _transactionStatus ) const
	{
		return _transactionStatus.m_token;
	}

	void operator()(CPending & _peding ) const
	{
		return _peding.m_token;
	}
};

template < class _Action >
class CGetTrackerInfo : public CResponseVisitorBase< _Action, node::NodeResponseList >
{
public:
	CGetTrackerInfo( _Action * const _action ):CResponseVisitorBase< _Action, node::NodeResponseList >( _action ){};

	void operator()(CTrackerStats & _transactionStatus ) const
	{
		return _transactionStatus;
	}
};

template < class _Action >
class CGetMediumError : public CResponseVisitorBase< _Action, node::NodeResponseList >
{
public:
	CGetMediumError( _Action * const _action ):CResponseVisitorBase< _Action, node::NodeResponseList >( _action ){};

	void operator()(CSystemError & _systemError ) const
    {
        return _systemError.m_errorType;
    }
};

template < class _Action >
class CGetBalance : public CResponseVisitorBase< _Action, node::NodeResponseList >
{
public:
	CGetBalance( _Action * const _action ):CResponseVisitorBase< _Action, node::NodeResponseList >( _action ){};

	void operator()(CAvailableCoins & _availableCoins ) const
	{
		return _availableCoins.m_availableCoins;
	}
};

void 
CSetResponseVisitor< node::NodeResponses >::visit( node::CSendTransactionAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< node::CSendTransactionAction, node::NodeResponseList > const &)CGetTransactionStatus< node::CSendTransactionAction >( &_action ), m_requestResponse );
//	_action.setTransactionToken(boost::apply_visitor( (CResponseVisitorBase< uint256 > const &)CGetToken(), m_requestRespond ));
}

void
CSetResponseVisitor< node::NodeResponses >::visit( node::CConnectAction & _action )
{
//	_action.setInProgressToken(boost::apply_visitor( (CResponseVisitorBase< uint256 > const &)CGetToken(), m_requestRespond ));
//	_action.setTrackerInfo(boost::apply_visitor( (CResponseVisitorBase< CTrackerStats > const &)CGetTrackerInfo(), m_requestRespond ));
//    _action.setMediumError(boost::apply_visitor( (CResponseVisitorBase< ErrorType::Enum > const &)CGetMediumError(), m_requestRespond ));
}

void
CSetResponseVisitor< node::NodeResponses >::visit( node::CSendBalanceInfoAction & _action )
{
//	_action.setBalance(boost::apply_visitor( (CResponseVisitorBase< std::map< uint256, CCoins > > const &)CGetBalance(), m_requestRespond ));
//	_action.setInProgressToken(boost::apply_visitor( (CResponseVisitorBase< uint256 > const &)CGetToken(), m_requestRespond ));
}

void
CSetResponseVisitor< node::NodeResponses >::visit( CAction< node::NodeResponses > & _action )
{

}


}
