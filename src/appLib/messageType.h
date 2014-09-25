#ifndef MESSAGE_TYPE_H
#define MESSAGE_TYPE_H

#include <QString>

namespace dims
{

QString const ServerName = "/tmp/dimsClient";

struct CMessageKind
{
	enum Enum
	{
		  Expectations
		, ErrorIndicator
		, Transaction
	};
};

struct CAppError
{
	enum Enum
	{
		  RefusedByClient
		, NotEnoughFunds
	};
};


}
#endif // MESSAGE_TYPE_H
