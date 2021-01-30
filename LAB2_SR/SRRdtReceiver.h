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
	deque<RcvMark>* WindowBuf;		//���ڻ���

public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:
	void receive(const Packet& packst);	//���ձ��ģ�����NetworkService����
};
#endif

