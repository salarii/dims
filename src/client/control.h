// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CLIENT_CONTROL_H
#define CLIENT_CONTROL_H

#include <boost/statechart/state_machine.hpp>
#include <boost/signals2.hpp>

#include <vector>
#include "wallet.h"

class AddressTableModel;

class CTransaction;
class uint256;
namespace client
{

struct CUninitiatedClient;

struct CClientSignals
{
	boost::signals2::signal<void( uint256 const &, CTransaction const & )> m_transactionAddmited;
	boost::signals2::signal<int(unsigned int)> m_messageboxPaymentRequest;
	boost::signals2::signal<void ( unsigned int )> m_updateTotalBalance;
	boost::signals2::signal<void ( CTransaction const & )> m_putTransactionIntoModel;
	boost::signals2::signal<void ( uint256 const & )> m_updateTransactionInModel;
};


class CClientControl : public boost::statechart::state_machine< CClientControl, CUninitiatedClient >
{
public:
	static CClientControl* getInstance();

	void setAddressTableModel( AddressTableModel * _addressTableModel );

	std::vector< std::string > getAvailableAddresses() const;

	void updateTotalBalance( int64_t _totalBalance );

	void addTransactionToModel( CTransaction const & _transaction );

	bool executePaymentMessageBox( unsigned int _cost);

	void transactionAddmited( uint256 const & _hash, CTransaction const & _transaction );

	CClientSignals & acquireClientSignals();

	bool isClientReady()const{ return m_clientReady; }

	bool setClientReady(){ m_clientReady = true; }

private:
	CClientControl();
private:
	bool m_clientReady;

	static CClientControl * ms_instance;

	AddressTableModel * m_addressTableModel;// invent  something  else  to  do such a  things, signals??

	CClientSignals m_clientSignals;
};

}

#endif // CLIENT_CONTROL_H
