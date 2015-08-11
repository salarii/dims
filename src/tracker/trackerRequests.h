#ifndef TRACKER_REQUESTS_H
#define TRACKER_REQUESTS_H

#include "common/medium.h"
#include "common/request.h"

#include "configureTrackerActionHandler.h"

#include "tracker/trackerFilters.h"

#include "core.h"

namespace tracker
{

class CDeliverInfoRequest : public common::CRequest< common::CTrackerTypes >
{
public:
		CDeliverInfoRequest( uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter );

		virtual void accept( common::CTrackerBaseMedium * _medium ) const;

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

class  CValidateTransactionsRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CValidateTransactionsRequest( std::vector< CTransaction > const & _transactions, common::CTrackerMediumFilter * _mediumFilter );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	std::vector< CTransaction > const & getTransactions() const;
private:
	std::vector< CTransaction > const m_transactions;
};


// one  template  for  all below

class  CPassMessageRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CPassMessageRequest( common::CMessage const & _message, uint256 const & _actionKey, CPubKey const & _prevKey, common::CTrackerMediumFilter * _mediumFilter );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	common::CMessage const & getMessage() const;

	uint256 getActionKey() const;

	CPubKey const & getPreviousKey() const;
private:
	uint256 const m_actionKey;

	common::CMessage m_message;

	CPubKey const m_prevKey;
};

class CTransactionsPropagationRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CTransactionsPropagationRequest( std::vector< CTransaction > const & _transactions, uint256 const & _actionKey,common::CTrackerMediumFilter * _mediumFilter );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	uint256 getActionKey() const;

	std::vector< CTransaction > const & getTransactions() const;
private:
	uint256 const m_actionKey;

	std::vector< CTransaction > const m_transactions;
};

// most likely temporary solution
class  CTransactionsStatusRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CTransactionsStatusRequest( CBundleStatus::Enum _bundleStatus, uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	uint256 getActionKey() const;

	CBundleStatus::Enum getBundleStatus() const;
private:
	uint256 const m_actionKey;

	CBundleStatus::Enum m_bundleStatus;
};

class CSetBloomFilterRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CSetBloomFilterRequest( CBloomFilter const & _bloomFilter );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	CBloomFilter const & getBloomFilter() const;
private:
	CBloomFilter const m_bloomFilter;
};

class CConnectToTrackerRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CConnectToTrackerRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	std::string getAddress() const;

	CAddress getServiceAddress() const;
private:
	std::string const m_trackerAddress;

	CAddress const m_serviceAddress;
};

struct CSpecificMediumFilter;

class CGetBalanceRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CGetBalanceRequest( uint160 const & _key );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	uint160 getKey() const;
private:
	uint160 m_key;
};

class CAskForRegistrationRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CAskForRegistrationRequest( uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
};

// not final version
class CRegisterProofRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CRegisterProofRequest( uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
};

class CTransactionConditionRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CTransactionConditionRequest( uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
};

class CTransactionAsClientRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CTransactionAsClientRequest( CTransaction const & _transaction, uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	uint256 getActionKey() const;

	CTransaction getTransaction() const;
private:
	CTransaction m_transaction;

	uint256 const m_actionKey;
};

}

#endif // TRACKER_REQUESTS_H
