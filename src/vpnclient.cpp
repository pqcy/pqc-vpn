#include "vpnclient.h"

VpnClient::VpnClient(QObject* parent) : GStateObj(parent) {
}

VpnClient::~VpnClient() {
	close();
}

bool VpnClient::doOpen() {
	GIntf* intf = GNetInfo::instance().intfList().findByName(realIntfName_);
	if (intf == nullptr) {
		SET_ERR(GErr::ObjectIsNull, QString("intf(%1)null").arg(realIntfName_));
		return false;
	}
	GIp ip = intf->ip();
	if (ip == 0) {
		SET_ERR(GErr::ValueIsNotZero, QString("ip is zero(%1)").arg(realIntfName_));
		return false;
	}

	if (intf->isSameLanIp(serverIp_)) {
		runCommand(QString("sudo route add -net %1 netmask 255.255.255.255 dev %3").arg(QString(serverIp_)).arg(realIntfName_));
	} else {
		GIp gateway = intf->gateway();
		runCommand(QString("sudo route add -net %1 gw %2 netmask 255.255.255.255 dev %3").arg(QString(serverIp_), QString(gateway)).arg(realIntfName_));

	}
	runCommand(QString("sudo ifconfig %1 down").arg(dummyIntfName_));
	runCommand(QString("sudo ip link set %1 addr %2").arg(dummyIntfName_).arg(dummyIntfMac_));
	runCommand(QString("sudo ifconfig %1 up").arg(dummyIntfName_));

	sockClient_.localIp_ = ip;
	sockClient_.localPort_ = 0;
	sockClient_.ip_ = serverIp_;
	sockClient_.port_ = serverPort_;
	if (!sockClient_.open()) {
		err = sockClient_.err;
		return false;
	}

	dummyPcapDevice_.intfName_ = dummyIntfName_;
	dummyPcapDevice_.mtu_ = 1500;
	if (!dummyPcapDevice_.open()) {
		err = dummyPcapDevice_.err;
		return false;
	}

	socketWrite_.intfName_ = dummyIntfName_;
	if (!socketWrite_.open()) {
		err = socketWrite_.err;
		return false;
	}

	std::thread* thread = new std::thread(&VpnClient::dhcpAndAddRouteTable, this);
	thread->detach();

	GThreadMgr::suspendStart();
	captureAndSendThread_.start();
	readAndReplyThread_.start();
	GThreadMgr::resumeStart();

	return true;
}

bool VpnClient::doClose() {
	qDebug() << "";

	runCommand(QString("sudo route del -net %1 netmask 255.255.255.255").arg(QString(serverIp_)));
	runCommand(QString("sudo ifconfig %1 down").arg(dummyIntfName_));

	sockClient_.close();
	dummyPcapDevice_.close();
	socketWrite_.close();

	captureAndSendThread_.quit();
	captureAndSendThread_.wait();
	readAndReplyThread_.quit();
	readAndReplyThread_.wait();

	return true;
}

void VpnClient::runCommand(QString program, bool sync) {
	qDebug() << program;
	int res;
	if (sync) {
		QProcess p;
		res = p.execute(program);
	} else {
		res = QProcess::startDetached(program);
	}
	if (res < 0) {
		qWarning() << QString("run %1 %2 return %3").arg(program).arg(res);
	}
}

void VpnClient::dhcpAndAddRouteTable() {
	runCommand(QString("sudo dhclient -i %1").arg(dummyIntfName_));
	runCommand(QString("echo dhclient finished"));
}

void VpnClient::CaptureAndSendThread::run() {
	qDebug() << "beg";
	VpnClient* client = PVpnClient(parent());
	GSyncPcapDevice* dummyPcapDevice = &client->dummyPcapDevice_;
#ifdef SUPPORT_VPN_TLS
	TlsClient* sockClient = &client->sockClient_;
#else // SUPPORT_VPN_TLS
	TcpClient* sockClient = &client->sockClient_;
#endif // SUPPORT_VPN_TLS

	while (client->active()) {
		GEthPacket packet;
		GPacket::Result res = dummyPcapDevice->read(&packet);
		if (res == GPacket::None) continue;
		if (res == GPacket::Eof || res == GPacket::Fail) break;

		GIpHdr* ipHdr = packet.ipHdr_;
		if (ipHdr == nullptr) continue;

		GTcpHdr* tcpHdr = packet.tcpHdr_;
		if (tcpHdr != nullptr)
			tcpHdr->sum_ = htons(GTcpHdr::calcChecksum(ipHdr, tcpHdr));
		GUdpHdr* udpHdr = packet.udpHdr_;
		if (udpHdr != nullptr)
			udpHdr->sum_ = htons(GUdpHdr::calcChecksum(ipHdr, udpHdr));
		ipHdr->sum_ = htons(GIpHdr::calcChecksum(ipHdr));

		uint16_t len = sizeof(GEthHdr) + ipHdr->len();
		char buf[MaxBufSize];
		memcpy(buf, "PQ", 2);
		*reinterpret_cast<uint16_t*>(&buf[2]) = htons(len);
		memcpy(&buf[4], packet.buf_.data_, len);

		int writeLen = sockClient->write(buf, 4 + len);
		if (writeLen == -1) break;
		// qDebug() << QString("session write %1").arg(len); // gilgil temp 2022.12.10
	}
	qDebug() << "end";
}

void VpnClient::ReadAndReplyThread::run() {
	qDebug() << "beg";
	VpnClient* client = PVpnClient(parent());
#ifdef SUPPORT_VPN_TLS
	TlsClient* sockClient = &client->sockClient_;
#else // SUPPORT_VPN_TLS
	TcpClient* sockClient = &client->sockClient_;
#endif // SUPPORT_VPN_TLS
	GSyncPcapDevice* dummyPcapDevice = &client->dummyPcapDevice_;
	GRawIpSocketWrite* socketWrite = &client->socketWrite_;

	while (client->active()) {
		char buf[MaxBufSize];
		int readLen = sockClient->readAll(buf, 4); // header size
		if (readLen != 4) break;
		if (buf[0] != 'P' || buf[1] != 'Q') {
			qWarning() << QString("invalid header %1 %2").arg(uint32_t(buf[0])).arg(uint32_t(buf[1]));
			break;
		}
		uint16_t len = ntohs(*reinterpret_cast<uint16_t*>(&buf[2]));
		if (len > 10000) {
			qWarning() << "too big len" << len;
		}
		readLen = sockClient->readAll(buf, len);
		if (readLen != len) {
			qWarning() << QString("readLen=%1 len=%2").arg(readLen).arg(len);
			break;
		}

		GEthPacket packet;
		packet.clear();
		packet.buf_.data_ = pbyte(buf);
		packet.buf_.size_ = len;
		packet.parse();

		bool isDhcp = false;
		GUdpHdr* udpHdr = packet.udpHdr_;
		if (udpHdr != nullptr) {
			uint16_t sport = udpHdr->sport();
			uint16_t dport = udpHdr->dport();
			if(sport == 68 || dport == 68) // DHCP
				isDhcp = true;
		}
		GPacket::Result res;
		if (isDhcp)
			res = dummyPcapDevice->write(&packet);
		else
			res = socketWrite->write(&packet);
		if (res != GPacket::Ok) {
			qWarning() << QString("pcapDevice_.write(&packet) return %1").arg(int(res));
		}
		// qWarning() << QString("pcap write %1").arg(packet.buf_.size_); // gilgil temp 2022.12.10
	}
	qDebug() << "end";
	emit client->closed();
}
