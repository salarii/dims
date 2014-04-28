#ifndef RESPONSE_VISITOR_INTERNAL_H
#define RESPONSE_VISITOR_INTERNAL_H

#include "visitorConfigurationUtilities.h"

namespace common
{

template < class _Action >
class CResponseVisitorBase : public boost::static_visitor< void >
{
public:
	CResponseVisitorBase( _Action const * _action ):m_action( _action ){};

	virtual void operator()(VisitorParam( 0 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 1 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 2 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 3 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 4 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 5 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 6 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 7 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 8 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 9 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 10 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 11 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 12 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 13 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 14 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 15 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 16 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 17 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 18 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 19 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( 20 )  & _param ) const
	{
	}

protected:
	_Action * const m_action;
};

}

#endif // RESPONSE_VISITOR_INTERNAL_H
