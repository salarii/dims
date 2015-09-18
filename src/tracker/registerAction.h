// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/action.h"

#include <boost/statechart/state_machine.hpp>

#ifndef REGISTER_ACTION_H
#define REGISTER_ACTION_H

namespace tracker
{

struct CInitiateRegistration;

class CRegisterAction : public common::CAction, public  boost::statechart::state_machine< CRegisterAction, CInitiateRegistration >
{
public:
	CRegisterAction( uint256 const & _actionKey, uintptr_t _nodePtr );

	CRegisterAction( uintptr_t _nodePtr );

	virtual void accept( common::CSetResponseVisitor & _visitor );

	uintptr_t getNodePtr() const { return m_nodePtr; }

	~CRegisterAction(){};

	void setRegisterPayment( int64_t _payment ){ m_registerPayment = _payment; }

	int64_t getRegisterPayment() const{ return m_registerPayment; }

	CPubKey getPublicKey() const;
private:
	int64_t m_registerPayment;

	uintptr_t m_nodePtr;
};

}

#endif // REGISTER_ACTION_H
