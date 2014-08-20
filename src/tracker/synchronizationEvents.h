#ifndef SYNCHRONIZATION_EVENTS_H
#define SYNCHRONIZATION_EVENTS_H

#include <boost/statechart/event.hpp>

namespace tracker
{

struct CSwitchToSynchronizing : boost::statechart::event< CSwitchToSynchronizing >
{
};

struct CSwitchToSynchronized : boost::statechart::event< CSwitchToSynchronized >
{
};

struct CAssistRequestEvent : boost::statechart::event< CAssistRequestEvent >
{
};

struct CGetNextBlockEvent : boost::statechart::event< CGetNextBlockEvent >
{
};


}

#endif // SYNCHRONIZATION_EVENTS_H
