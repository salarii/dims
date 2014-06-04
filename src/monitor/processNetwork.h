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
