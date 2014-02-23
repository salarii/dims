#ifndef MANAGE_NETWORK_H
#define MANAGE_NETWORK_H 

class CManageNetwork
{
public:
	CManageNetwork();

	mainLoop();
private:
	void connectToNetwork();
	void negotiateWithMonitor();
// pointer to network task queue
};


#endif // MANAGE_NETWORK_H