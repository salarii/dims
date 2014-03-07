

#ifndef TASK_H
#define TASK_H

namespace Self
{

class CTask
{
public: 
	virtual getPriority() = 0;
	virtual bool execute() = 0;
	virtual bool skipNow() = 0;
};


}

#endif