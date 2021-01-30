#include "Global.h"
#include "GBNRdtReceiver.h"
FILE* Receiver_seqnum = fopen("D:\\VisualStudioFile\\LAB2_TCP\\Receiver_seqnum.txt", "w");
GBNRdtReceiver::GBNRdtReceiver() :expectedSeqnumRcvd(0) {
	lastAckPkt.acknum = (expectedSeqnumRcvd + 7) % 8;			//约定以希望收到的序号的前一个数表示nck
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';

	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

GBNRdtReceiver::~GBNRdtReceiver() {}

void GBNRdtReceiver::receive(const Packet& packet) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);

	//如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一直
	if (checkSum == packet.checksum && this->expectedSeqnumRcvd == packet.seqnum) {
		pUtils->printPacket("接收方正确收到发送方报文", packet);
		fprintf(Receiver_seqnum, "\n接收方正确收到发送方报文，seqnum=%d\n", packet.seqnum);

		//取出message，向上递交给应用层
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);

		lastAckPkt.acknum = packet.seqnum;	//确认序号等于收到的报文的序号
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("接收方发送正确报文", lastAckPkt);
		fprintf(Receiver_seqnum, "\n接收方发送正确报文，确认号为：%d\n", lastAckPkt.acknum);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境

		++expectedSeqnumRcvd %= 8;
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("接收方没有正确收到发送方的报文，校验和错误", packet);
			fprintf(Receiver_seqnum, "\n接收方收到发送方错误的校验和\n");
		}
		else {
			pUtils->printPacket("接收方没有正确收到发送方的报文，报文序号不对", packet);
			fprintf(Receiver_seqnum, "\n接收方收到发送方错误序号为%d的报文\n", packet.seqnum);
		}
		pUtils->printPacket("接收方重新发送上次的确认报文", lastAckPkt);
		fprintf(Receiver_seqnum, "接收方重新发送上次的确认报文，确认序号为：%d", lastAckPkt.acknum);

		pns->sendToNetworkLayer(SENDER, lastAckPkt);
	}
}