#include"Global.h"
#include"SRRdtSender.h"
#include<iostream>
FILE* Sender_seqnum = fopen("D:\\VisualStudioFile\\LAB_SR\\Sender_seqnum.txt", "w");

SRRdtSender::SRRdtSender():sendbase(0),nextseqnum(0),N(4),seqsize(8),Window(new deque<SenderMark>){}

SRRdtSender::~SRRdtSender() {
	if (Window) {
		delete Window;
		Window = 0;
	}
}

bool SRRdtSender::getWaitingState() {
	if (Window->size() == N)
		return true;
	return false;
}

bool SRRdtSender::send(const Message& message) {
	if (this->getWaitingState())
		return false;
	SenderMark sndmrk;
	sndmrk.isrcved = false;
	sndmrk.pkt.acknum = -1;
	sndmrk.pkt.seqnum=nextseqnum;
	sndmrk.pkt.checksum = 0;
	memcpy(sndmrk.pkt.payload, message.data, sizeof(message.data));
	sndmrk.pkt.checksum = pUtils->calculateCheckSum(sndmrk.pkt);
	Window->push_back(sndmrk);
	pUtils->printPacket("发送方发送报文：", sndmrk.pkt);
	fprintf(Sender_seqnum, "\n发送方发送报文，seqnum=%d\n", sndmrk.pkt.seqnum);
	//SR的发送方为每一个没有接受到ACK的分组设置一个定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, sndmrk.pkt.seqnum);
	pns->sendToNetworkLayer(RECEIVER, sndmrk.pkt);
	++nextseqnum %= seqsize;
	return true;
}

void SRRdtSender::receive(const Packet& ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	int oft = (ackPkt.acknum - sendbase + seqsize) % seqsize;
	if (checkSum == ackPkt.checksum && oft<N&&Window->at(oft).isrcved==false){
		pns->stopTimer(SENDER, ackPkt.acknum);	//不是停止sendbase序号的分组，注意区分GBN
		
		(Window->at(oft)).isrcved = true;	//标记正确接受的报文

		pUtils->printPacket("发送方正确收到确认：", ackPkt);
		fprintf(Sender_seqnum, "\n发送方接受到确认号为%d的报文\n", ackPkt.acknum);
		if (Window->size() && Window->begin()->isrcved == true) {
			cout << "发送方滑动窗口前，sendbase=" << sendbase << ", nextseqnum= " << nextseqnum << ", windowsize=" << Window->size() << endl;
			fprintf(Sender_seqnum, "\n发送方滑动窗口前,sendbase=%d, nextseqnum=%d, windowsize=%d\n", sendbase, nextseqnum, Window->size());
			/*输出滑动窗口前，窗口的内容*/
			/*？？？*/
			//滑动窗口
			while (Window->size() && Window->begin()->isrcved == true) {
				//如果收到的报文的序号是发送窗口的基序号（在前面会将基序号标志为true），则滑动窗口
				++sendbase %= seqsize;
				Window->pop_front();
			}
			cout << "发送方滑动窗口后，sendbase=" << sendbase << ", nextseqnum=" << nextseqnum << ", windowsize=" << Window->size()<<endl;
			fprintf(Sender_seqnum, "\n发送方滑动窗口后，snedbase=%d, nextseqnum=%d, windowsize=%d\n", sendbase, nextseqnum, Window->size());
		}
	}
}

void SRRdtSender::timeoutHandler(int seqnum) {
	cout<<"超时，发送方重新发送超时的报文,报文的序号为："<<seqnum<<endl;
	fprintf(Sender_seqnum, "\n超时，发送方重新发送超时的报文，报文的序号为：%d\n", seqnum);
	pns->stopTimer(SENDER, seqnum);
	//只重发超时的报文
	int oft = (seqnum - sendbase + seqsize) % seqsize;
	if (oft < Window->size()) {	
		//如果超时的报文在当前的窗口范围内,重发，否则忽略
		pns->sendToNetworkLayer(RECEIVER, (Window->at(oft)).pkt);
		pUtils->printPacket("发送方定时器时间到，重新发送报文: ", (Window->at(oft)).pkt);
		pns->startTimer(SENDER, Configuration::TIME_OUT, seqnum);
	}
}
