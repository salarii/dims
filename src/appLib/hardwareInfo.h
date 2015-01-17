// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef HARDWARE_INFO_H
#define HARDWARE_INFO_H

#include <vector>
namespace dims
{

struct CHardwareNumbers
{
	std::vector< unsigned char > m_macHash;
	std::vector< unsigned char > m_volumeHash;
	std::vector< unsigned char > m_cpuHash;
};

CHardwareNumbers getHardwareNumbers();

}
#endif // HARDWARE_INFO_H
