#ifndef  SR_RDT_SENDER_H
#define SR_RDT_SENDER_H
#include "RdtSender.h"
#include <queue>
using namespace std;
struct SenderMark {		//��Ƿ���Ϊ�ѽ���
	bool isrcved;
	Packet pkt;
};
class SRRdtSender :public RdtSender {
private:
	int sendbase;
	int nextseqnum;
	const int N;
	const int seqsize;
	deque<SenderMark>* Window;
public:
	bool getWaitingState();
	bool send(const Message& message);						//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(const Packet& ackPkt);						//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkServiceSimulator����

	SRRdtSender();
	virtual ~SRRdtSender();
};


#endif