#include "Global.h"
#include "GBNRdtReceiver.h"
FILE* Receiver_seqnum = fopen("D:\\VisualStudioFile\\LAB2_TCP\\Receiver_seqnum.txt", "w");
GBNRdtReceiver::GBNRdtReceiver() :expectedSeqnumRcvd(0) {
	lastAckPkt.acknum = (expectedSeqnumRcvd + 7) % 8;			//Լ����ϣ���յ�����ŵ�ǰһ������ʾnck
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';

	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

GBNRdtReceiver::~GBNRdtReceiver() {}

void GBNRdtReceiver::receive(const Packet& packet) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);

	//���У�����ȷ��ͬʱ�յ����ĵ���ŵ��ڽ��շ��ڴ��յ��ı������һֱ
	if (checkSum == packet.checksum && this->expectedSeqnumRcvd == packet.seqnum) {
		pUtils->printPacket("���շ���ȷ�յ����ͷ�����", packet);
		fprintf(Receiver_seqnum, "\n���շ���ȷ�յ����ͷ����ģ�seqnum=%d\n", packet.seqnum);

		//ȡ��message�����ϵݽ���Ӧ�ò�
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);

		lastAckPkt.acknum = packet.seqnum;	//ȷ����ŵ����յ��ı��ĵ����
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("���շ�������ȷ����", lastAckPkt);
		fprintf(Receiver_seqnum, "\n���շ�������ȷ���ģ�ȷ�Ϻ�Ϊ��%d\n", lastAckPkt.acknum);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷��

		++expectedSeqnumRcvd %= 8;
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı��ģ�У��ʹ���", packet);
			fprintf(Receiver_seqnum, "\n���շ��յ����ͷ������У���\n");
		}
		else {
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı��ģ�������Ų���", packet);
			fprintf(Receiver_seqnum, "\n���շ��յ����ͷ��������Ϊ%d�ı���\n", packet.seqnum);
		}
		pUtils->printPacket("���շ����·����ϴε�ȷ�ϱ���", lastAckPkt);
		fprintf(Receiver_seqnum, "���շ����·����ϴε�ȷ�ϱ��ģ�ȷ�����Ϊ��%d", lastAckPkt.acknum);

		pns->sendToNetworkLayer(SENDER, lastAckPkt);
	}
}