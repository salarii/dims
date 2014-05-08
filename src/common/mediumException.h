// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MEDIUM_EXCEPTION_H
#define MEDIUM_EXCEPTION_H

#include <exception>
#include "errorResponse.h"

namespace common
{

// for time beeing this  will be common for all mediums, consider how to change this behavior??
struct CMediumException : public std::exception
{
public:
	CMediumException(ErrorType::Enum _error):m_error(_error){};
	ErrorType::Enum m_error;
};


}


#endif // MEDIUM_EXCEPTION_H
