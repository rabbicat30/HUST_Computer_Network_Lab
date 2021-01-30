#ifndef  GBN_RDT_SENDER_H
#define GBN_RDT_SENDER_H
#include "RdtSender.h"
#include <queue>
using namespace std;
class GBNRdtSender :public RdtSender {
private:
	int base;		//�����--����δȷ�Ϸ������
	int nextseqnum;		//��һ��������������
	const int N;		//�ѷ��͵�δȷ�ϵķ�����������Ϊ4
	const int seqsize;		//��Ŵ�С��������λ�����������룬��0-7
	deque<Packet>* Window;		//ʹ��˫�����ʵ�ִ���

	int  redundancy_ack;		//��gbn�Ļ������������ack�ĸ���

public:
	bool getWaitingState();
	bool send(const Message& message);						//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(const Packet& ackPkt);						//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkServiceSimulator����

	GBNRdtSender();
	virtual ~GBNRdtSender();
};
#endif // ! GBN_RDT_SENDER_H

