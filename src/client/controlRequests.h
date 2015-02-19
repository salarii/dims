// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONTROL_REQUESTS_H
#define CONTROL_REQUESTS_H

#include "configureNodeActionHadler.h"
#include "common/request.h"

namespace client
{

class CDnsInfoRequest : public common::CRequest< NodeResponses >
{
public:
   CDnsInfoRequest();

   virtual void accept( common::CMedium< NodeResponses > * _medium ) const;

   virtual common::CMediumFilter< NodeResponses > * getMediumFilter() const;
};

class CRecognizeNetworkRequest : public common::CRequest< NodeResponses >
{
public:
   CRecognizeNetworkRequest();

   virtual void accept( common::CMedium< NodeResponses > * _medium ) const;

   virtual common::CMediumFilter< NodeResponses > * getMediumFilter() const;
};

}

#endif // CONTROL_REQUESTS_H
