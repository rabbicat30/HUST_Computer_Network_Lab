#include "Global.h"
#include "GBNRdtSender.h"
#include<iostream>
FILE* Sender_seqnum = fopen("D:\\VisualStudioFile\\LAB2_TCP\\Sender_seqnum.txt", "w");
GBNRdtSender::GBNRdtSender() :N(4), seqsize(8), base(0), nextseqnum(0), redundancy_ack(0),Window(new deque<Packet>) {};

GBNRdtSender::~GBNRdtSender() {
	if (Window) {
		delete Window;
		Window = 0;
	}
}

bool GBNRdtSender::getWaitingState() {
	if (Window->size() == N)		//每次要发送一个报文就将该报文放入队列中，当队列的大小为窗口最大容量时，窗口满了
		return true;
	return false;
}

bool GBNRdtSender::send(const Message& message) {
	if (this->getWaitingState())		//如果发送方正处于等待确认状态
		return false;
	//检查可以发送后，发送： sndpkt[nextseqnum] = make_pkt(nextseqnum,data,chksum)
	Packet newpkt;			//要发送的报文
	newpkt.acknum = -1;		//忽略该字段
	newpkt.seqnum = nextseqnum;	//实验要求报文段序号为以报文段为段位进行编号而不是数据字节的个数，所以这里不用改
	newpkt.checksum = 0;
	memcpy(newpkt.payload, message.data, sizeof(message.data));
	newpkt.checksum = pUtils->calculateCheckSum(newpkt);
	Window->push_back(newpkt);		//将已经发送的报文加入窗口
	pUtils->printPacket("发送方发送报文: ", newpkt);	//显示包括seqnum，acknum，checknum，内容
	fprintf(Sender_seqnum, "发送方发送报文,seqnum = %d\n", newpkt.seqnum);	//将控制台输出的序号信息重定向到文件中
	if (base == nextseqnum)
		pns->startTimer(SENDER, Configuration::TIME_OUT, newpkt.seqnum);
	pns->sendToNetworkLayer(RECEIVER, newpkt);
	++nextseqnum %= seqsize;		//循环序号
	return true;
}

void GBNRdtSender::receive(const Packet& ackPkt) {
	//此时不能现判断是否处于等待状态，因为比如说第一次发送肯定是没满的，就不满足
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	if (checkSum == ackPkt.checksum){
		if (ackPkt.acknum == (base + 7) % 8) {
			//如果收到的确认号时前一个的确认号
			redundancy_ack++;
			printf("\n收到冗余的ack：%d\n", ackPkt.acknum);
			fprintf(Sender_seqnum, "\n收到冗余的ack: %d\n", ackPkt.acknum);
			if (redundancy_ack == 3) {
				//如果收到了3个冗余的ack，发送方认为当前发送的报文段丢失，在超之前重传
				redundancy_ack = 0;
				//当前已经发送但还没确认的报文位于发送窗口的base位置
				printf("\n收到3个冗余ack，开始快速重传，重传报文序号为：%d\n", Window->begin()->seqnum);
				fprintf(Sender_seqnum, "\n收到3个冗余ack，开始快速重传，重传报文序号为：%d\n", Window->begin()->seqnum);
				pns->stopTimer(SENDER, Window->begin()->seqnum);
				pns->sendToNetworkLayer(RECEIVER, *Window->begin());
				pns->startTimer(SENDER, Configuration::TIME_OUT, Window->begin()->seqnum);
			}
		}
		else if (ackPkt.acknum != (base + 7) % 8) {
			redundancy_ack = 0;			//收到非冗余的ack，清空计数器！
			pns->stopTimer(SENDER, base);			//停止窗口第一个报文的定时器

			pUtils->printPacket("发送方正确收到确认: ", ackPkt);
			cout << "滑动窗口前，base=" << base << "，nextseqnum=" << nextseqnum << "，windowsize=" << Window->size() << endl;
			fprintf(Sender_seqnum, "滑动窗口前，base=%d，nextseqnum=%d，windowsize=%d\n\n", base, nextseqnum, Window->size());

			//滑动窗口
			while (base != (ackPkt.acknum + 1) % seqsize) {		//确保收到的不是重复的ACK
				Window->pop_front();	//窗口队列中弹出一个
				++base %= seqsize;
			}
			cout << "\n滑动窗口后，base=" << base << "，nextsequnm=" << nextseqnum << ", windowsize=" << Window->size() << endl;
			fprintf(Sender_seqnum, "\n滑动窗口后，base=%d，nextseqnum=%d，windowsize=%d\n\n", base, nextseqnum, Window->size());
			if (Window->size())			//还有可以发送的报文
				pns->startTimer(SENDER, Configuration::TIME_OUT, Window->front().seqnum);//重启计时器
		}
	}
}

void GBNRdtSender::timeoutHandler(int seqnum) {
	printf("超时，重新发送窗口报文, 此时窗口： base = % d，nextseqnum = % d，windowsize = % d\n", base, nextseqnum, Window->size());
	fprintf(Sender_seqnum, "超时，重新发送窗口报文,此时窗口： base=%d，nextseqnum=%d，window\size=%d\n", base, nextseqnum, Window->size());
	redundancy_ack = 0;
	pns->stopTimer(SENDER, seqnum);
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqnum);
	//TCP这里不要重发base到nextseqnum-1，只要发认为超时的那个就可以！
	if (Window->size())
		pns->sendToNetworkLayer(RECEIVER, *Window->begin());
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqnum);
}


