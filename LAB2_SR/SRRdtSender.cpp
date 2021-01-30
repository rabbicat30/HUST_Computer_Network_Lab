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
	pUtils->printPacket("���ͷ����ͱ��ģ�", sndmrk.pkt);
	fprintf(Sender_seqnum, "\n���ͷ����ͱ��ģ�seqnum=%d\n", sndmrk.pkt.seqnum);
	//SR�ķ��ͷ�Ϊÿһ��û�н��ܵ�ACK�ķ�������һ����ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, sndmrk.pkt.seqnum);
	pns->sendToNetworkLayer(RECEIVER, sndmrk.pkt);
	++nextseqnum %= seqsize;
	return true;
}

void SRRdtSender::receive(const Packet& ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	int oft = (ackPkt.acknum - sendbase + seqsize) % seqsize;
	if (checkSum == ackPkt.checksum && oft<N&&Window->at(oft).isrcved==false){
		pns->stopTimer(SENDER, ackPkt.acknum);	//����ֹͣsendbase��ŵķ��飬ע������GBN
		
		(Window->at(oft)).isrcved = true;	//�����ȷ���ܵı���

		pUtils->printPacket("���ͷ���ȷ�յ�ȷ�ϣ�", ackPkt);
		fprintf(Sender_seqnum, "\n���ͷ����ܵ�ȷ�Ϻ�Ϊ%d�ı���\n", ackPkt.acknum);
		if (Window->size() && Window->begin()->isrcved == true) {
			cout << "���ͷ���������ǰ��sendbase=" << sendbase << ", nextseqnum= " << nextseqnum << ", windowsize=" << Window->size() << endl;
			fprintf(Sender_seqnum, "\n���ͷ���������ǰ,sendbase=%d, nextseqnum=%d, windowsize=%d\n", sendbase, nextseqnum, Window->size());
			/*�����������ǰ�����ڵ�����*/
			/*������*/
			//��������
			while (Window->size() && Window->begin()->isrcved == true) {
				//����յ��ı��ĵ�����Ƿ��ʹ��ڵĻ���ţ���ǰ��Ὣ����ű�־Ϊtrue�����򻬶�����
				++sendbase %= seqsize;
				Window->pop_front();
			}
			cout << "���ͷ��������ں�sendbase=" << sendbase << ", nextseqnum=" << nextseqnum << ", windowsize=" << Window->size()<<endl;
			fprintf(Sender_seqnum, "\n���ͷ��������ں�snedbase=%d, nextseqnum=%d, windowsize=%d\n", sendbase, nextseqnum, Window->size());
		}
	}
}

void SRRdtSender::timeoutHandler(int seqnum) {
	cout<<"��ʱ�����ͷ����·��ͳ�ʱ�ı���,���ĵ����Ϊ��"<<seqnum<<endl;
	fprintf(Sender_seqnum, "\n��ʱ�����ͷ����·��ͳ�ʱ�ı��ģ����ĵ����Ϊ��%d\n", seqnum);
	pns->stopTimer(SENDER, seqnum);
	//ֻ�ط���ʱ�ı���
	int oft = (seqnum - sendbase + seqsize) % seqsize;
	if (oft < Window->size()) {	
		//�����ʱ�ı����ڵ�ǰ�Ĵ��ڷ�Χ��,�ط����������
		pns->sendToNetworkLayer(RECEIVER, (Window->at(oft)).pkt);
		pUtils->printPacket("���ͷ���ʱ��ʱ�䵽�����·��ͱ���: ", (Window->at(oft)).pkt);
		pns->startTimer(SENDER, Configuration::TIME_OUT, seqnum);
	}
}
