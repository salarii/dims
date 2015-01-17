// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PROCESS_NETWORK_H
#define PROCESS_NETWORK_H

namespace common
{
class CSelfNode;

}

class CDataStream;

namespace monitor
{

class CProcessNetwork
{
public:
	bool sendMessages(common::CSelfNode* pto, bool fSendTrickle);

	bool processMessage(common::CSelfNode* pfrom, CDataStream& vRecv);

	bool processMessages(common::CSelfNode* pfrom);

	static CProcessNetwork* getInstance();
private:
	static CProcessNetwork * ms_instance;
};


}

#endif // PROCESS_NETWORK_H
