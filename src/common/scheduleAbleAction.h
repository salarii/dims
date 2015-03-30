#ifndef SCHEDULE_ABLE_ACTION_H
#define SCHEDULE_ABLE_ACTION_H

namespace common
{

template < class _Type >
class CScheduleAbleAction : public CAction< _Type >
{
public:
		CScheduleAbleAction ()
		{
			m_actionKey = getRandNumber();
		}

		uint256 getActionKey() const
		{
			return m_actionKey;
		}

		~CCommunicationAction()
		{}
	protected:
		uint256 m_actionKey;
};

}





#endif // SCHEDULE_ABLE_ACTION_H
