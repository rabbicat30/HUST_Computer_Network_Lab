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
		//��ʼ�����ջ��崰�ڣ���СΪ4
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
	//���У����Ƿ���ȷ
	int checksum = pUtils->calculateCheckSum(packet);
	int oft = (packet.seqnum - rcvbase + seqsize) % seqsize;
	//���У�����ȷ���յ��ı��ĵ����λ�ڴ�����
	if (checksum == packet.checksum && oft<N) {
		pUtils->printPacket("���շ���ȷ�յ����ͷ�����", packet);
		fprintf(Receiver_seqnum, "\n���շ���ȷ�յ����ͷ����ģ�seqnum=%d\n", packet.seqnum);

		//ȡ��message�����浽���մ���
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		printf("���շ��������Ϊ%d�ķ���\n", packet.seqnum);
		fprintf(Receiver_seqnum, "\n���շ��������Ϊ%d�ķ���\n", packet.seqnum);
		WindowBuf->at(oft).msg = msg;
		WindowBuf->at(oft).isack = true;

		//���ͶԵ�ǰ�����ACK
		rcvpkt.acknum = packet.seqnum;	//ȷ�Ϻ�Ϊ�յ���������
		rcvpkt.checksum = pUtils->calculateCheckSum(rcvpkt);
		pUtils->printPacket("���շ�������ȷ����", rcvpkt);
		fprintf(Receiver_seqnum, "\n���շ�������ȷ����,ȷ�Ϻ�Ϊ��%d\n", rcvpkt.acknum);
		pns->sendToNetworkLayer(SENDER, rcvpkt);

		//�������ķ�������Ϊrcvbase�����ͷ����ڻ���
		//�������ǰ���ڵ���Ϣ
		if (WindowBuf->begin()->isack) {
			printf("���շ���������ǰ�����շ��յ��ķ��������У�");
			fprintf(Receiver_seqnum, "���շ���������ǰ�����շ��յ��ķ���������: ");
			for (deque<RcvMark>::iterator itor = WindowBuf->begin(); itor != WindowBuf->end(); itor++) {
				if ((*itor).isack) {
					//����ָ��ǰ�����ָ������׸�����ָ���ƫ�Ƽ������
					printf("%d ", (itor - WindowBuf->begin() + rcvbase + seqsize) % seqsize);
					fprintf(Receiver_seqnum, "%d ", (itor - WindowBuf->begin() + rcvbase + seqsize) % seqsize);
				}

			}
			printf("\n");
			fprintf(Receiver_seqnum, "\n");

			//����rcvbase��ʼ�����ķ���һ���Ͻ����ϲ㣬Ȼ�󴰿ڰ��ո������ƶ�
			printf("���շ���������ǰ��rcvbase= %d\n", rcvbase);
			fprintf(Receiver_seqnum, "\n���շ���������ǰ��rcvbase=%d\n", rcvbase);
			while (WindowBuf->begin()->isack) {
				//���Ͻ�������
				pns->delivertoAppLayer(RECEIVER, WindowBuf->begin()->msg);
				WindowBuf->pop_front();
				//��������һ���µ�δ��־�ķ�����Ϊռλ
				RcvMark rcvmark;
				rcvmark.isack = false;
				WindowBuf->push_back(rcvmark);

				//base+1
				++rcvbase %= seqsize;
			}
			printf("���շ��������ں�rcvbase=%d\n", rcvbase);
			fprintf(Receiver_seqnum, "\n���շ��������ں�rcvbase=%d\n", rcvbase);
		}
	}
	else if (checksum == packet.checksum&&oft>=N) {
		//����������λ��ǰһ�����ڣ�ֻ����ȷ��
		rcvpkt.acknum = packet.seqnum;	//ȷ�Ϻ�Ϊ�յ���������
		rcvpkt.checksum = pUtils->calculateCheckSum(rcvpkt);
		pUtils->printPacket("���շ���������ǰһ�����ڵı���", rcvpkt);
		fprintf(Receiver_seqnum, "\n���շ���������ǰһ�����ڵı���,ȷ�Ϻ�Ϊ��%d\n", rcvpkt.acknum);
		pns->sendToNetworkLayer(SENDER, rcvpkt);
	}
	else {
		//��������������յ��ķ���
		pUtils->printPacket("���շ��յ�����ı���", packet);
		fprintf(Receiver_seqnum, "\n���շ��յ�����ı���\n");
	}
}