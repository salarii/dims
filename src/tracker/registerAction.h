// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REGISTER_ACTION_H
#define REGISTER_ACTION_H

#include <boost/statechart/state_machine.hpp>

#include "key.h"

#include "common/action.h"

namespace tracker
{

struct CInitiateRegistration;

class CRegisterAction : public common::CAction, public  boost::statechart::state_machine< CRegisterAction, CInitiateRegistration >
{
public:
	CRegisterAction( uint256 const & _actionKey, CPubKey const & _partnerKey );

	CRegisterAction( CPubKey const & _partnerKey );

	virtual void accept( common::CSetResponseVisitor & _visitor );

	CPubKey getPartnerKey()const
	{
		return m_partnerKey;
	}

	~CRegisterAction(){};

	void setRegisterPayment( int64_t _payment ){ m_registerPayment = _payment; }

	int64_t getRegisterPayment() const{ return m_registerPayment; }

	void setInNetwork( bool _flag );
private:
	int64_t m_registerPayment;

	CPubKey m_partnerKey;
};

}

#endif // REGISTER_ACTION_H
