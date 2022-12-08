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
		if (ipHdr == nullptr) continue;
		uint16_t len = sizeof(GEthHdr) + ipHdr->len();
		qDebug() << "len=" << len; // gilgil temp

		char buf[MaxBufSize];
		memcpy(buf, "PQ", 2);
		*reinterpret_cast<uint16_t*>(&buf[2]) = htons(len);
		memcpy(&buf[4], packet.buf_.data_, len);

		int writeLen = tcpClient->write(buf, 4 + len);
		if (writeLen == -1) break;
		qWarning() << QString("session write %1").arg(4 + len);
	}
	qDebug() << ""; // gilgil temp 2022.12.07
}

void VpnClient::ReadAndReplyThread::run() {
	qDebug() << ""; // gilgil temp 2022.12.07
	VpnClient* client = PVpnClient(parent());
	TcpClient* tcpClient = &client->tcpClient_;
	GRawIpSocketWrite* socketWrite = &client->socketWrite_;

	while (true) {
		char buf[MaxBufSize];
		int readLen = tcpClient->readAll(buf, 4); // header size
		if (readLen != 4) break;
		if (buf[0] != 'P' || buf[1] != 'Q') {
			qWarning() << QString("invalid header %1 %2").arg(uint32_t(buf[0])).arg(uint32_t(buf[1]));
			break;
		}
		uint16_t len = ntohs(*reinterpret_cast<uint16_t*>(&buf[2]));
		qDebug() << "len=" << len; // gilgil temp 2022.12.08
		if (len > 10000) {
			qWarning() << "too big len" << len;
		}
		readLen = tcpClient->readAll(buf, len);
		if (readLen != len) {
			qWarning() << QString("readLen=%1 len=%2").arg(readLen).arg(len);
			break;
		}

		GEthPacket packet;
		packet.buf_.data_ = pbyte(buf);
		packet.buf_.size_ = len;
		packet.parse();
		//GPacket::Result res = socketWrite->write(&packet);
		GPacket::Result res = client->dummyPcapDevice_.write(&packet);
		if (res != GPacket::Ok) {
			qWarning() << QString("pcapDevice_.write(&packet) return %d").arg(int(res));
		}
		qWarning() << QString("pcap write %1").arg(packet.buf_.size_);
	}
	qDebug() << ""; // gilgil temp 2022.12.07
}
