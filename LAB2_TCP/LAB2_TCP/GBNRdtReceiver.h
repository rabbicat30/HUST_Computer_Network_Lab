#ifndef GBN_RDT_RECEIVER_H
#define GBN_RDT_RECEIVER_H
#include "RdtReceiver.h"
class GBNRdtReceiver :public RdtReceiver {
	int expectedSeqnumRcvd;
	Packet lastAckPkt;
public:
	GBNRdtReceiver();
	virtual ~GBNRdtReceiver();

public:
	void receive(const Packet& packst);	//接收报文，将被NetworkService调用
};
#endif

