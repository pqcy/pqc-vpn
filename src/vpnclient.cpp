#include "vpnclient.h"

VpnClient::VpnClient(QObject* parent) : GStateObj(parent) {
}

VpnClient::~VpnClient() {
	close();
}

bool VpnClient::doOpen() {
	if (!tcpClient_.open()) {
		err = tcpClient_.err;
		return false;
	}

	if (!dummyPcapDevice_.open()) {
		err = dummyPcapDevice_.err;
		return false;
	}

	if (!socketWrite_.open()) {
		err = socketWrite_.err;
		return false;
	}

	captureAndSendThread_.start();
	readAndReplyThread_.start();

	return true;
}

bool VpnClient::doClose() {
	tcpClient_.close();
	dummyPcapDevice_.close();
	socketWrite_.close();

	captureAndSendThread_.quit();
	captureAndSendThread_.wait();
	readAndReplyThread_.quit();
	readAndReplyThread_.wait();

	return true;
}

void VpnClient::CaptureAndSendThread::run() {
	qDebug() << ""; // gilgil temp 2022.12.07
	VpnClient* client = PVpnClient(parent());
	GSyncPcapDevice* dummyPcapDevice = &client->dummyPcapDevice_;
	TcpClient* tcpClient = &client->tcpClient_;

	while (client->active()) {
		GEthPacket packet;
		GPacket::Result res = dummyPcapDevice->read(&packet);
		if (res == GPacket::Eof || res == GPacket::Fail) break;
		if (res == GPacket::None) continue;

		GIpHdr* ipHdr = packet.ipHdr_;
		if (packet.ipHdr_ == nullptr) continue;
		uint16_t len = sizeof(GEthHdr) + ipHdr->len();
		qDebug() << "len=" << len; // gilgil temp

		char buf[MaxBufSize];
		memcpy(buf, "PQ", 2);
		*reinterpret_cast<uint16_t*>(&buf[2]) = htons(len);
		memcpy(&buf[4], packet.buf_.data_, len);

		int writelLen = tcpClient->write(buf, 4 + len);
		if (writelLen == -1) break;
	}
	qDebug() << ""; // gilgil temp 2022.12.07
}

void VpnClient::ReadAndReplyThread::run() {
	qDebug() << ""; // gilgil temp 2022.12.07
	qDebug() << ""; // gilgil temp 2022.12.07
}
