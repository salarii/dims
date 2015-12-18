// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef INIT_H
#define INIT_H

#include <boost/thread.hpp>

class CWallet;

extern std::string strWalletFile;
extern CWallet* pwalletMain;

namespace monitor
{

void StartShutdown();

bool AppInit(boost::thread_group& threadGroup);
void Shutdown();
}

#endif // INIT_H
