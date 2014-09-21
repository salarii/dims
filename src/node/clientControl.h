// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CLIENT_CONTROL_H
#define CLIENT_CONTROL_H

#include <boost/statechart/state_machine.hpp>
#include <boost/signals2.hpp>

#include <vector>

class AddressTableModel;

class CTransaction;
class uint256;
namespace client
{

struct CUninitiatedClient;

struct CClientSignals
{
	boost::signals2::signal<void ( unsigned int )> m_updateTotalBalance;
	boost::signals2::signal<void ( CTransaction const & _transaction )> m_putTransactionIntoModel;
	boost::signals2::signal<void ( uint256 const & _transaction )> m_updateTransactionInModel;
//	boost::signals2::signal<void (  )> m_sendTransaction;
};


class CClientControl : public boost::statechart::state_machine< CClientControl, CUninitiatedClient >
{
public:
	static CClientControl* getInstance();

	void setAddressTableModel( AddressTableModel * _addressTableModel );

	std::vector< std::string > getAvailableAddresses() const;

	void updateTotalBalance( int64_t _totalBalance );

	void addTransactionToModel( CTransaction const & _transaction );

	CClientSignals & acquireClientSignals();
private:
	CClientControl();
private:
	static CClientControl * ms_instance;

	AddressTableModel * m_addressTableModel;

	CClientSignals m_clientSignals;
};

}

#endif // CLIENT_CONTROL_H
