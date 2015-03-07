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

		virtual common::CTrackerMediumFilter * getMediumFilter() const;
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


class CAskForTransactionsRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CAskForTransactionsRequest( std::vector< uint256 > const & _blockHashes, common::CTrackerMediumFilter * _mediumFilter );

	virtual void accept( common::CTrackerBaseMedium * m_mediumNumber ) const;

	virtual common::CTrackerMediumFilter * getMediumFilter() const;

	std::vector< uint256 > const & getBlockHashes() const;
private:
	std::vector< uint256 > const m_blockHashes;

};


class CSetBloomFilterRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CSetBloomFilterRequest( CBloomFilter const & _bloomFilter );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	virtual common::CTrackerMediumFilter * getMediumFilter() const;

	CBloomFilter const & getBloomFilter() const;
private:
	CBloomFilter const m_bloomFilter;
};

class CConnectToTrackerRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CConnectToTrackerRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	virtual common::CTrackerMediumFilter * getMediumFilter() const;

	std::string getAddress() const;

	CAddress getServiceAddress() const;
private:
	std::string const m_trackerAddress;

	CAddress const m_serviceAddress;
};

struct CSpecificMediumFilter;
struct CDiskBlock;

class CGetSynchronizationInfoRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CGetSynchronizationInfoRequest( uint256 const & _actionKey, uint64_t _timeStamp );

	CGetSynchronizationInfoRequest( uint256 const & _actionKey, uint64_t _timeStamp, common::CTrackerMediumFilter * _mediumFilter );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	virtual common::CTrackerMediumFilter * getMediumFilter() const;

	uint256 getActionKey() const;

	uint64_t getTimeStamp() const;
private:
	uint256 const m_actionKey;

	uint64_t const m_timeStamp;
};

class CGetNextBlockRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CGetNextBlockRequest( uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter, int _blockKind );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	virtual common::CTrackerMediumFilter * getMediumFilter() const;

	uint256 getActionKey() const;

	int getBlockKind() const;
private:
	uint256 const m_actionKey;

	int m_blockKind;
};

template < class Block >
class CSetNextBlockRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CSetNextBlockRequest( uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter, Block * _discBlock, unsigned int _blockIndex );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	virtual common::CTrackerMediumFilter * getMediumFilter() const;

	uint256 getActionKey() const;

	Block * getBlock() const;

	unsigned int getBlockIndex() const;
private:
	uint256 const m_actionKey;

	Block * m_discBlock;

	unsigned int m_blockIndex;
};

template < class Block >
CSetNextBlockRequest< Block >::CSetNextBlockRequest( uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter, Block * _discBlock, unsigned int _blockIndex )
	: common::CRequest< common::CTrackerTypes >( _mediumFilter )
	, m_actionKey( _actionKey )
	, m_discBlock( _discBlock )
	, m_blockIndex( _blockIndex )
{
}

template < class Block >
void
CSetNextBlockRequest< Block >::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

template < class Block >
common::CTrackerMediumFilter *
CSetNextBlockRequest< Block >::getMediumFilter() const
{
	return common::CRequest< common::CTrackerTypes >::m_mediumFilter;
}

template < class Block >
uint256
CSetNextBlockRequest< Block >::getActionKey() const
{
	return m_actionKey;
}

template < class Block >
Block *
CSetNextBlockRequest< Block >::getBlock() const
{
	return m_discBlock;
}

template < class Block >
unsigned int
CSetNextBlockRequest< Block >::getBlockIndex() const
{
	return m_blockIndex;
}


class CGetBalanceRequest : public common::CRequest< common::CTrackerTypes >
{
public:
	CGetBalanceRequest( uint160 const & _key );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	virtual common::CTrackerMediumFilter * getMediumFilter() const;

	uint160 getKey() const;
private:
	uint160 m_key;
};

}

#endif // TRACKER_REQUESTS_H
