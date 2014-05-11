#ifndef INIT_H
#define INIT_H

#include <boost/thread.hpp>

class CWallet;

extern std::string strWalletFile;
extern CWallet* pwalletMain;

namespace node
{

bool AppInit1(boost::thread_group& threadGroup);
void Shutdown();
}

#endif // INIT_H
