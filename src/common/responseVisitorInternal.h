#ifndef RESPONSE_VISITOR_INTERNAL_H
#define RESPONSE_VISITOR_INTERNAL_H

#include "visitorConfigurationUtilities.h"

#include <boost/variant.hpp>
namespace common
{

template < class _Action, typename _ParamList >
class CResponseVisitorBase : public boost::static_visitor< void >
{
public:
	CResponseVisitorBase( _Action const * _action ):m_action( _action ){};
/*
	virtual void operator()(VisitorParam(  _ParamList, 0 )  & _param ) const
	{
	}
*/
	typename

	boost::mpl::if_<
	typename boost::mpl::less<
		  boost::mpl::int_< 0 >
		  , typename boost::mpl::size< _ParamList >::type >::type
		  , typename boost::mpl::at< _ParamList , typename boost::mpl::if_< typename boost::mpl::less< boost::mpl::int_< 0 >::type, typename boost::mpl::size< _ParamList >::type >::type, boost::mpl::int_< 0 >::type, boost::mpl::int_< 0 > >::type >::type
	,boost::mpl::at< DummyList
	,boost::mpl::int_< 0 > >::type  >::type  stefi;
/*
	virtual void operator()(VisitorParam( typename _ParamList, 0 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 1 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 2 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 3 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 4 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 5 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 6 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 7 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 8 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 9 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 10 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 11 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 12 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 13 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 14 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 15 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 16 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 17 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 18 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 19 )  & _param ) const
	{
	}

	virtual void operator()(VisitorParam( _ParamList, 20 )  & _param ) const
	{
	}
*/
protected:
	_Action * const m_action;
};

}

#endif // RESPONSE_VISITOR_INTERNAL_H
