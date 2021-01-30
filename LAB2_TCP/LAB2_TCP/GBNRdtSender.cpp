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
	if (Window->size() == N)		//ÿ��Ҫ����һ�����ľͽ��ñ��ķ�������У������еĴ�СΪ�����������ʱ����������
		return true;
	return false;
}

bool GBNRdtSender::send(const Message& message) {
	if (this->getWaitingState())		//������ͷ������ڵȴ�ȷ��״̬
		return false;
	//�����Է��ͺ󣬷��ͣ� sndpkt[nextseqnum] = make_pkt(nextseqnum,data,chksum)
	Packet newpkt;			//Ҫ���͵ı���
	newpkt.acknum = -1;		//���Ը��ֶ�
	newpkt.seqnum = nextseqnum;	//ʵ��Ҫ���Ķ����Ϊ�Ա��Ķ�Ϊ��λ���б�Ŷ����������ֽڵĸ������������ﲻ�ø�
	newpkt.checksum = 0;
	memcpy(newpkt.payload, message.data, sizeof(message.data));
	newpkt.checksum = pUtils->calculateCheckSum(newpkt);
	Window->push_back(newpkt);		//���Ѿ����͵ı��ļ��봰��
	pUtils->printPacket("���ͷ����ͱ���: ", newpkt);	//��ʾ����seqnum��acknum��checknum������
	fprintf(Sender_seqnum, "���ͷ����ͱ���,seqnum = %d\n", newpkt.seqnum);	//������̨����������Ϣ�ض����ļ���
	if (base == nextseqnum)
		pns->startTimer(SENDER, Configuration::TIME_OUT, newpkt.seqnum);
	pns->sendToNetworkLayer(RECEIVER, newpkt);
	++nextseqnum %= seqsize;		//ѭ�����
	return true;
}

void GBNRdtSender::receive(const Packet& ackPkt) {
	//��ʱ�������ж��Ƿ��ڵȴ�״̬����Ϊ����˵��һ�η��Ϳ϶���û���ģ��Ͳ�����
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
	if (checkSum == ackPkt.checksum){
		if (ackPkt.acknum == (base + 7) % 8) {
			//����յ���ȷ�Ϻ�ʱǰһ����ȷ�Ϻ�
			redundancy_ack++;
			printf("\n�յ������ack��%d\n", ackPkt.acknum);
			fprintf(Sender_seqnum, "\n�յ������ack: %d\n", ackPkt.acknum);
			if (redundancy_ack == 3) {
				//����յ���3�������ack�����ͷ���Ϊ��ǰ���͵ı��Ķζ�ʧ���ڳ�֮ǰ�ش�
				redundancy_ack = 0;
				//��ǰ�Ѿ����͵���ûȷ�ϵı���λ�ڷ��ʹ��ڵ�baseλ��
				printf("\n�յ�3������ack����ʼ�����ش����ش��������Ϊ��%d\n", Window->begin()->seqnum);
				fprintf(Sender_seqnum, "\n�յ�3������ack����ʼ�����ش����ش��������Ϊ��%d\n", Window->begin()->seqnum);
				pns->stopTimer(SENDER, Window->begin()->seqnum);
				pns->sendToNetworkLayer(RECEIVER, *Window->begin());
				pns->startTimer(SENDER, Configuration::TIME_OUT, Window->begin()->seqnum);
			}
		}
		else if (ackPkt.acknum != (base + 7) % 8) {
			redundancy_ack = 0;			//�յ��������ack����ռ�������
			pns->stopTimer(SENDER, base);			//ֹͣ���ڵ�һ�����ĵĶ�ʱ��

			pUtils->printPacket("���ͷ���ȷ�յ�ȷ��: ", ackPkt);
			cout << "��������ǰ��base=" << base << "��nextseqnum=" << nextseqnum << "��windowsize=" << Window->size() << endl;
			fprintf(Sender_seqnum, "��������ǰ��base=%d��nextseqnum=%d��windowsize=%d\n\n", base, nextseqnum, Window->size());

			//��������
			while (base != (ackPkt.acknum + 1) % seqsize) {		//ȷ���յ��Ĳ����ظ���ACK
				Window->pop_front();	//���ڶ����е���һ��
				++base %= seqsize;
			}
			cout << "\n�������ں�base=" << base << "��nextsequnm=" << nextseqnum << ", windowsize=" << Window->size() << endl;
			fprintf(Sender_seqnum, "\n�������ں�base=%d��nextseqnum=%d��windowsize=%d\n\n", base, nextseqnum, Window->size());
			if (Window->size())			//���п��Է��͵ı���
				pns->startTimer(SENDER, Configuration::TIME_OUT, Window->front().seqnum);//������ʱ��
		}
	}
}

void GBNRdtSender::timeoutHandler(int seqnum) {
	printf("��ʱ�����·��ʹ��ڱ���, ��ʱ���ڣ� base = % d��nextseqnum = % d��windowsize = % d\n", base, nextseqnum, Window->size());
	fprintf(Sender_seqnum, "��ʱ�����·��ʹ��ڱ���,��ʱ���ڣ� base=%d��nextseqnum=%d��window\size=%d\n", base, nextseqnum, Window->size());
	redundancy_ack = 0;
	pns->stopTimer(SENDER, seqnum);
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqnum);
	//TCP���ﲻҪ�ط�base��nextseqnum-1��ֻҪ����Ϊ��ʱ���Ǹ��Ϳ��ԣ�
	if (Window->size())
		pns->sendToNetworkLayer(RECEIVER, *Window->begin());
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqnum);
}


