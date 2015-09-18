#ifndef TRACKER_REQUESTS_H
#define TRACKER_REQUESTS_H

#include "common/medium.h"
#include "common/request.h"

#include "tracker/filters.h"

#include "core.h"

namespace tracker
{

class CDeliverInfoRequest : public common::CRequest
{
public:
		CDeliverInfoRequest( uint256 const & _actionKey, common::CMediumFilter * _mediumFilter );

		virtual void accept( common::CMedium * _medium ) const;

		uint256 const & getActionKey() const
		{
			return m_actionKey;
		}
	private:
		uint256 const m_actionKey;
};


struct CBundleStatus
{
	enum Enum
	{
		  Ack
		, NotValid
		, Known
	};
};

class CMediumClassFilter;

class  CValidateTransactionsRequest : public common::CRequest
{
public:
	CValidateTransactionsRequest( std::vector< CTransaction > const & _transactions, common::CMediumFilter * _mediumFilter );

	virtual void accept( common::CMedium * _medium ) const;

	std::vector< CTransaction > const & getTransactions() const;
private:
	std::vector< CTransaction > const m_transactions;
};


// one  template  for  all below

class  CPassMessageRequest : public common::CRequest
{
public:
	CPassMessageRequest( common::CMessage const & _message, uint256 const & _actionKey, CPubKey const & _prevKey, common::CMediumFilter * _mediumFilter );

	virtual void accept( common::CMedium * _medium ) const;

	common::CMessage const & getMessage() const;

	uint256 getActionKey() const;

	CPubKey const & getPreviousKey() const;
private:
	uint256 const m_actionKey;

	common::CMessage m_message;

	CPubKey const m_prevKey;
};

class CTransactionsPropagationRequest : public common::CRequest
{
public:
	CTransactionsPropagationRequest( std::vector< CTransaction > const & _transactions, uint256 const & _actionKey,common::CMediumFilter * _mediumFilter );

	virtual void accept( common::CMedium * _medium ) const;

	uint256 getActionKey() const;

	std::vector< CTransaction > const & getTransactions() const;
private:
	uint256 const m_actionKey;

	std::vector< CTransaction > const m_transactions;
};

// most likely temporary solution
class  CTransactionsStatusRequest : public common::CRequest
{
public:
	CTransactionsStatusRequest( CBundleStatus::Enum _bundleStatus, uint256 const & _actionKey, common::CMediumFilter * _mediumFilter );

	virtual void accept( common::CMedium * _medium ) const;

	uint256 getActionKey() const;

	CBundleStatus::Enum getBundleStatus() const;
private:
	uint256 const m_actionKey;

	CBundleStatus::Enum m_bundleStatus;
};

class CConnectToTrackerRequest : public common::CRequest
{
public:
	CConnectToTrackerRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress );

	virtual void accept( common::CMedium * _medium ) const;

	std::string getAddress() const;

	CAddress getServiceAddress() const;
private:
	std::string const m_trackerAddress;

	CAddress const m_serviceAddress;
};

struct CSpecificMediumFilter;

class CGetBalanceRequest : public common::CRequest
{
public:
	CGetBalanceRequest( uint160 const & _key );

	virtual void accept( common::CMedium * _medium ) const;

	uint160 getKey() const;
private:
	uint160 m_key;
};

class CAskForRegistrationRequest : public common::CRequest
{
public:
	CAskForRegistrationRequest( uint256 const & _actionKey, common::CMediumFilter * _mediumFilter );

	virtual void accept( common::CMedium * _medium ) const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
};

}

#endif // TRACKER_REQUESTS_H
