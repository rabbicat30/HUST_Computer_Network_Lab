#include"Global.h"
#include"SRRdtReceiver.h"

FILE* Receiver_seqnum = fopen("D:\\VisualStudioFile\\LAB_SR\\Receiver_seqnum.txt", "w");

SRRdtReceiver::SRRdtReceiver() :rcvbase(0), N(4), seqsize(8), WindowBuf(new deque < RcvMark>) {
	rcvpkt.acknum = 0;
	rcvpkt.checksum = 0;
	rcvpkt.seqnum = -1;
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++)
		rcvpkt.payload[i] = '.';
	rcvpkt.checksum = pUtils->calculateCheckSum(rcvpkt);
	for (int i = 0; i < N; i++) {
		//初始化接收缓冲窗口，大小为4
		RcvMark rcvmark;
		rcvmark.isack = false;
		WindowBuf->push_back(rcvmark);
	}
}

SRRdtReceiver::~SRRdtReceiver(){
	if (WindowBuf) {
		delete WindowBuf;
		WindowBuf = 0;
	}
}

void SRRdtReceiver::receive(const Packet& packet) {
	//检查校验和是否正确
	int checksum = pUtils->calculateCheckSum(packet);
	int oft = (packet.seqnum - rcvbase + seqsize) % seqsize;
	//如果校验和正确且收到的报文的序号位于窗口内
	if (checksum == packet.checksum && oft<N) {
		pUtils->printPacket("接收方正确收到发送方报文", packet);
		fprintf(Receiver_seqnum, "\n接收方正确收到发送方报文，seqnum=%d\n", packet.seqnum);

		//取出message，缓存到接收窗口
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		printf("接收方缓存序号为%d的分组\n", packet.seqnum);
		fprintf(Receiver_seqnum, "\n接收方缓存序号为%d的分组\n", packet.seqnum);
		WindowBuf->at(oft).msg = msg;
		WindowBuf->at(oft).isack = true;

		//发送对当前分组的ACK
		rcvpkt.acknum = packet.seqnum;	//确认号为收到分组的序号
		rcvpkt.checksum = pUtils->calculateCheckSum(rcvpkt);
		pUtils->printPacket("接收方发送正确报文", rcvpkt);
		fprintf(Receiver_seqnum, "\n接收方发送正确报文,确认号为：%d\n", rcvpkt.acknum);
		pns->sendToNetworkLayer(SENDER, rcvpkt);

		//如果到达的分组的序号为rcvbase，发送方窗口滑动
		//输出滑动前窗口的信息
		if (WindowBuf->begin()->isack) {
			printf("接收方滑动窗口前，接收方收到的分组的序号有：");
			fprintf(Receiver_seqnum, "接收方滑动窗口前，接收方收到的分组的序号有: ");
			for (deque<RcvMark>::iterator itor = WindowBuf->begin(); itor != WindowBuf->end(); itor++) {
				if ((*itor).isack) {
					//利用指向当前分组的指针基于首个分组指针的偏移计算序号
					printf("%d ", (itor - WindowBuf->begin() + rcvbase + seqsize) % seqsize);
					fprintf(Receiver_seqnum, "%d ", (itor - WindowBuf->begin() + rcvbase + seqsize) % seqsize);
				}

			}
			printf("\n");
			fprintf(Receiver_seqnum, "\n");

			//将从rcvbase开始连续的分组一起上交给上层，然后窗口按照该数量移动
			printf("接收方滑动窗口前，rcvbase= %d\n", rcvbase);
			fprintf(Receiver_seqnum, "\n接收方滑动窗口前，rcvbase=%d\n", rcvbase);
			while (WindowBuf->begin()->isack) {
				//向上交付分组
				pns->delivertoAppLayer(RECEIVER, WindowBuf->begin()->msg);
				WindowBuf->pop_front();
				//必须设置一个新的未标志的分组作为占位
				RcvMark rcvmark;
				rcvmark.isack = false;
				WindowBuf->push_back(rcvmark);

				//base+1
				++rcvbase %= seqsize;
			}
			printf("接收方滑动窗口后，rcvbase=%d\n", rcvbase);
			fprintf(Receiver_seqnum, "\n接收方滑动窗口后，rcvbase=%d\n", rcvbase);
		}
	}
	else if (checksum == packet.checksum&&oft>=N) {
		//如果分组序号位于前一个窗口，只发送确认
		rcvpkt.acknum = packet.seqnum;	//确认号为收到分组的序号
		rcvpkt.checksum = pUtils->calculateCheckSum(rcvpkt);
		pUtils->printPacket("接收方发送来自前一个窗口的报文", rcvpkt);
		fprintf(Receiver_seqnum, "\n接收方发送来自前一个窗口的报文,确认号为：%d\n", rcvpkt.acknum);
		pns->sendToNetworkLayer(SENDER, rcvpkt);
	}
	else {
		//其他情况，忽略收到的分组
		pUtils->printPacket("接收方收到错误的报文", packet);
		fprintf(Receiver_seqnum, "\n接收方收到错误的报文\n");
	}
}