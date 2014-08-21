#ifndef MEDIUM_KINDS_H
#define MEDIUM_KINDS_H

namespace common
{

struct CMediumKinds
{
	enum Enum
	{
		  Internal
		, BitcoinsNodes
		, DimsNodes // quarantine after this
		, Trackers
	};
};

}

#endif // MEDIUM_KINDS_H
