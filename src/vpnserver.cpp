#include "vpnserver.h"

VpnServer::VpnServer(QObject* parent) : TcpServer(parent) {
}

VpnServer::~VpnServer() {
	close();
}

bool VpnServer::doOpen() {
	if (!TcpServer::doOpen()) return false;

	if (!pcapDevice_.open()) {
		err = pcapDevice_.err;
		return false;
	}

	captureAndProcessThread_.start();

	return true;
}

bool VpnServer::doClose() {
	TcpServer::doClose();
	pcapDevice_.close();
	captureAndProcessThread_.quit();
	captureAndProcessThread_.wait();
	return true;
}

void VpnServer::CaptureAndProcessThread::run() {
	VpnServer* server = PVpnServer(parent());
}

void VpnServer::run(Session* session) {
	qDebug() << "";

	while (true) {
		char buf[MaxBufSize];
		int readLen = session->readAll(buf, 4); // header size
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

		readLen = session->read(buf, len);
		if (readLen != len) {
			qWarning() << QString("len=%1 readLen=%2").arg(len).arg(readLen);
			break;
		}

		GEthPacket packet;
		packet.buf_.data_ = pbyte(buf);
		packet.buf_.size_ = len;
		packet.parse();
		GPacket::Result res = pcapDevice_.write(&packet);
		if (res != GPacket::Ok) {
			qWarning() << QString("pcapDevice_.write return %1").arg(int(res));
			break;
		}
	}

	qDebug() << "";
}
