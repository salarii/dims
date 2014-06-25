// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACCEPT_NODE_ACTION_H
#define ACCEPT_NODE_ACTION_H

#include "common/action.h"
#include "configureSeedActionHandler.h"
#include <boost/statechart/state_machine.hpp>
#include <boost/optional.hpp>

namespace seed
{

struct CUninitiated;

class CAcceptNodeAction : public common::CAction< SeedResponses >, public  boost::statechart::state_machine< CAcceptNodeAction, CUninitiated >
{
public:
	CAcceptNodeAction( std::vector< unsigned char > const & _payload, unsigned int _mediumKind );

	CAcceptNodeAction( CAddress const & _nodeAddress );

	virtual common::CRequest< SeedResponses >* execute();

	virtual void accept( common::CSetResponseVisitor< SeedResponses > & _visitor );

	void setRequest( common::CRequest< SeedResponses >* _request );

	CAddress getAddress() const;

	std::vector< unsigned char > getPayload() const;

	void setMediumKind( unsigned int _mediumKind );
// not safe
	unsigned int getMediumKind() const;

	bool getValid() const{ return m_valid; }

	void setValid( bool _valid ){ m_valid = _valid; }
private:
	common::CRequest< SeedResponses >* m_request;
	CAddress const m_nodeAddress;

	static int const ms_randomPayloadLenght = 32;

	std::vector< unsigned char > m_payload;

	unsigned int m_mediumKind;

	bool const m_passive;

	bool m_valid;
};


}

#endif // ACCEPT_NODE_ACTION_H
