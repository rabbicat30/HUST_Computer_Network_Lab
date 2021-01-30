#ifndef SR_RDT_RECEIVER_H
#define SR_RDT_RECEIVER_H
#include"RdtReceiver.h"
#include<queue>
using namespace std;
struct RcvMark {
	bool isack;
	Message msg;
};
class SRRdtReceiver :public RdtReceiver {
	int rcvbase;
	const int N;
	const int seqsize;
	Packet rcvpkt;
	deque<RcvMark>* WindowBuf;		//用于缓存

public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:
	void receive(const Packet& packst);	//接收报文，将被NetworkService调用
};
#endif

