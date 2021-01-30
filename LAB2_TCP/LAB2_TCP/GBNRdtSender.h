#ifndef  GBN_RDT_SENDER_H
#define GBN_RDT_SENDER_H
#include "RdtSender.h"
#include <queue>
using namespace std;
class GBNRdtSender :public RdtSender {
private:
	int base;		//基序号--最早未确认分组序号
	int nextseqnum;		//下一个待发分组的序号
	const int N;		//已发送但未确认的分组数，建议为4
	const int seqsize;		//序号大小，建议三位二进制数编码，即0-7
	deque<Packet>* Window;		//使用双向队列实现窗口

	int  redundancy_ack;		//在gbn的基础上添加冗余ack的个数

public:
	bool getWaitingState();
	bool send(const Message& message);						//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(const Packet& ackPkt);						//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);					//Timeout handler，将被NetworkServiceSimulator调用

	GBNRdtSender();
	virtual ~GBNRdtSender();
};
#endif // ! GBN_RDT_SENDER_H

