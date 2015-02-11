// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PAYMENT_DATA_H
#define PAYMENT_DATA_H

#include "core.h"
#include "serialize.h"
#include "common/commonResponses.h"

namespace dims
{

struct CLicenseData
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_privateKey);
		READWRITE(m_macSignature);
		READWRITE(m_procSignature);
		READWRITE(m_volumeSignature);
		READWRITE(m_trasaction);
		READWRITE(m_trackerPubKey);
		READWRITE(m_monitorData);
		READWRITE(m_monitorPubKey);
		READWRITE(m_transactionStatusSignature);
	)

	CPrivKey m_privateKey;
	std::vector<unsigned char> m_macSignature;
	std::vector<unsigned char> m_procSignature;
	std::vector<unsigned char> m_volumeSignature;
	CTransaction m_trasaction;
	CPubKey m_trackerPubKey;// used tracker pub key
	CPubKey m_monitorPubKey;// used monitor pub key
	common::CMonitorData m_monitorData;
	// picked  tracker PubKey
	std::vector<unsigned char> m_transactionStatusSignature;
};

template < class Message >
struct CPackedMessage
{
	CPackedMessage( int _messageKind, Message const & _message ):m_messageKind( _messageKind ),m_message(_message)
	{}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_messageKind);
		READWRITE(m_message);
	)

	int m_messageKind;
	Message m_message;
};


struct CExpectationMessage
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_privateKey);
		READWRITE(m_targetId);
		READWRITE(m_value);
		READWRITE(m_trackers);
		READWRITE(m_monitors);
	)
	CPrivKey m_privateKey;
	CKeyID m_targetId;
	int64_t m_value;
	std::vector<CKeyID> m_trackers;
	std::vector<CKeyID> m_monitors;
};

struct CErrorIndication
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_errorKind);
	)
	int m_errorKind;
};

struct CProofTransaction
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_trasaction);
		READWRITE(m_transactionStatusSignature);
	)

	CTransaction m_trasaction;
	std::vector<unsigned char> m_transactionStatusSignature;
};


}



#endif // PAYMENT_DATA_H
