#ifndef SEED_BITCOIN_H_
#define SEED_BITCOIN_H_ 1

#include "protocol.h"

namespace seed
{

bool TestNode(const CService &cip, int &ban, int &client, std::string &clientSV, int &blocks, std::vector<CAddress>* vAddr);

}

#endif
