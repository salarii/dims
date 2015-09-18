// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CLIENT_STRUCT_H
#define CLIENT_STRUCT_H

namespace client
{

struct ClientMediums
{
	enum Enum
	{
		Trackers
		, Monitors
		, Seed
		, Time
		, NetworkInfo
		, Unknown
		, UndeterminedTrackers
	};

};

}

#endif // CLIENT_STRUCT_H
