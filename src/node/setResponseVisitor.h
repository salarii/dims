#ifndef SET_RESPONSE_VISITOR_H
#define SET_RESPONSE_VISITOR_H

namespace node
{

class CSetResponseVisitor
{
public:
	CSetResponseVisitor( RequestRespond _requestRespond );

	void visit( CSendTransaction & _sendTransaction );

	void visit( CAction & _action );
private:

	RequestRespond m_requestRespond;
};


}

#endif