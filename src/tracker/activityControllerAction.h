#ifndef ACTIVITY_CONTROLLER_ACTION_H
#define ACTIVITY_CONTROLLER_ACTION_H

#include "core.h"

#include "common/action.h"
#include <boost/statechart/state_machine.hpp>

namespace tracker
{

struct CActivitySatatus
{
	enum Enum
	{
		Active,
		Inactive
	};

};


struct CActivityInitial;

class CActivityControllerAction : public common::CAction, public boost::statechart::state_machine< CActivityControllerAction, CActivityInitial >
{
public:
	CActivityControllerAction( CPubKey const & _nodeKey, CAddress const & _address, CActivitySatatus::Enum _status );

	CActivityControllerAction( uint256 const & _actionKey );

	virtual void accept( common::CSetResponseVisitor & _visitor );
public:
	CPubKey m_nodeKey;
	CAddress m_address;
	CActivitySatatus::Enum m_status;
};

}

#endif // ACTIVITY_CONTROLLER_ACTION_H
