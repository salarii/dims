// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SELF_NODE_H
#define SELF_NODE_H

namespace Self
{

class CSelfNode : public CNode
{
public:
	CSelfNode();
	CKeyID getPubKeyId();
private:
	CPubKey m_pubKey;
};

}

#endif
