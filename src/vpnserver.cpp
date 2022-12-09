#include "vpnserver.h"
#include "net/pdu/getharppacket.h"

#ifdef SUPPORT_VPN_TLS
VpnServer::VpnServer(QObject* parent) : TlsServer(parent) {
}
#else // SUPPORT_VPN_TLS
VpnServer::VpnServer(QObject* parent) : TcpServer(parent) {
}
#endif // SUPPORT_VPN_TLS

VpnServer::~VpnServer() {
	close();
}

bool VpnServer::doOpen() {
#ifdef SUPPORT_VPN_TLS
	if (!TlsServer::doOpen()) return false;
#else // SUPPORT_VPN_TLS
	if (!TcpServer::doOpen()) return false;
#endif // SUPPORT_VPN_TLS

	pcapDevice_.intfName_ = intfName_;
	if (!pcapDevice_.open()) {
		err = pcapDevice_.err;
		return false;
	}

	arpPcapDevice_.intfName_ = intfName_;
	arpPcapDevice_.filter_ = "arp";
	if (!arpPcapDevice_.open()) {
		err = arpPcapDevice_.err;
		return false;
	}

	atm_.intfName_ = intfName_;
	if (!atm_.open()) {
		err = atm_.err;
		return false;
	}

	intf_ = GNetInfo::instance().intfList().findByName(intfName_);
	if (intf_ == nullptr) {
		QString msg = QString("can not find interface for %1").arg(intfName_);
		SET_ERR(GErr::ValueIsNull, msg);
		return false;
	}

	captureAndProcessThread_.start();
	arpResolveThread_.start();

	return true;
}

bool VpnServer::doClose() {
	TcpServer::doClose();
	pcapDevice_.close();
	atm_.close();
	captureAndProcessThread_.quit();
	captureAndProcessThread_.wait();
	arpResolveThread_.quit();
	arpResolveThread_.wait();
	return true;
}

void VpnServer::CaptureAndProcessThread::run() {
	VpnServer* server = PVpnServer(parent());
	GSyncPcapDevice* pcapDevice = &server->pcapDevice_;
	ClientInfoMap* cim = &server->cim_;
	GIntf* intf = server->intf_;

	while (server->active()) {
		GEthPacket packet;
		GPacket::Result res = pcapDevice->read(&packet);
		if (res == GPacket::Eof || res == GPacket::Fail) break;
		if (res == GPacket::None) continue;

		//QThread::sleep(1); // gilgil temp 2022.12.08

		GEthHdr* ethHdr = packet.ethHdr_;
		if (ethHdr == nullptr) continue;

		GIpHdr* ipHdr = packet.ipHdr_;
		if (ipHdr == nullptr) continue;

		GMac smac = ethHdr->smac();
		GMac dmac = ethHdr->dmac();

		if (smac == intf->mac() || dmac == intf->mac()) continue; // server interfacd packet
		{
			QMutexLocker ml(&cim->m_);
			if (cim->find(smac) != cim->end()) continue; // client sending packet
		}

		GTcpHdr* tcpHdr = packet.tcpHdr_;
		if (tcpHdr != nullptr)
			tcpHdr->sum_ = htons(GTcpHdr::calcChecksum(ipHdr, tcpHdr));
		GUdpHdr* udpHdr = packet.udpHdr_;
		if (udpHdr != nullptr)
			udpHdr->sum_ = htons(GUdpHdr::calcChecksum(ipHdr, udpHdr));
		ipHdr->sum_ = htons(GIpHdr::calcChecksum(ipHdr));

		uint16_t len = sizeof(GEthHdr) + ipHdr->len();
		qDebug() << QString("len=%1 smac=%2 dmac=%3").arg(len).arg(QString(smac)).arg(QString(dmac));

		char buf[MaxBufSize];
		memcpy(buf, "PQ", 2);
		*reinterpret_cast<uint16_t*>(&buf[2]) = htons(len);
		memcpy(&buf[4], packet.buf_.data_, len);

		if (dmac.isBroadcast() || dmac.isMulticast()) {
			QMutexLocker ml(&cim->m_);
			qDebug() << QString("broadcast or multicast %1").arg(cim->count()); // gilgil temp 2022.12.08
			for (ClientInfo* ci : *cim) {
				ci->session_->write(buf, 4 + len);
				qDebug() << QString("session write %1 %2").arg(4 + len).arg(QString(ci->mac_));
			}
		} else {
			QMutexLocker ml(&cim->m_);
			ClientInfoMap::iterator it = cim->find(dmac);
			if (it != cim->end()) {
				ClientInfo* ci = *it;
				Session* session = ci->session_;
				int writeLen = session->write(buf, 4 + len);
				if (writeLen == -1) break;
				qDebug() << QString("session write %1 to %2").arg(4 + len).arg(QString(ci->mac_));
			}
		}
	}
}

void VpnServer::ArpResolveThread::run() {
	qDebug() << "";
	VpnServer* server = PVpnServer(parent());
	GSyncPcapDevice* arpPcapDevice = &server->arpPcapDevice_;
	ClientInfoMap* cim = &server->cim_;
	while (server->active()) {
		GEthPacket packet;
		GPacket::Result res =arpPcapDevice->read(&packet);
		if (res == GPacket::Eof || res == GPacket::Fail) break;
		if (res == GPacket::None) continue;

		//QThread::sleep(1); // gilgil temp 2022.12.09

		GEthHdr* ethHdr = packet.ethHdr_;
		if (ethHdr == nullptr) continue;

		GArpHdr* arpHdr = packet.arpHdr_;
		if (arpHdr == nullptr) continue;

		if (arpHdr->op() != GArpHdr::Request) continue;
		{
			GIp tip = arpHdr->tip();
			QMutexLocker ml(&cim->m_);
			for (ClientInfo* ci : *cim) {
				if (ci->ip_ == tip) {
					GEthArpPacket sendPacket;
					sendPacket.init(ethHdr->smac(), ci->mac_, GArpHdr::Request, ci->mac_, ci->ip_, arpHdr->smac(), arpHdr->sip());
					GBuf buf(pbyte(&sendPacket), sizeof(sendPacket));
					GPacket::Result res = arpPcapDevice->write(buf);
					if (res != GPacket::Ok)
						qWarning() << QString("send arp packet return %1").arg(int(res));
				}
			}
		}
	}
	qDebug() << "";
}

void VpnServer::run(Session* session) {
	qDebug() << "";
	ClientInfo ci;
	ClientInfoMap::iterator it = cim_.end();

	while (active()) {
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
		readLen = session->readAll(buf, len);
		if (readLen != len) {
			qWarning() << QString("readLen=%1 len=%2").arg(readLen).arg(len);
			break;
		}

		//QThread::sleep(1); // gilgil temp 2022.12.08

		GEthPacket packet;
		packet.clear();
		packet.buf_.data_ = pbyte(buf);
		packet.buf_.size_ = len;
		packet.parse();

		GEthHdr* ethHdr = packet.ethHdr_;
		if (ethHdr == nullptr) continue;

		GIpHdr* ipHdr = packet.ipHdr_;
		if (ipHdr == nullptr) continue;

		GMac smac = ethHdr->smac();
		GMac dmac = ethHdr->dmac();
		if (smac == dmac) {
			GIp ip = ipHdr->dip();
			ip = intf_->getAdjIp(ip);
			GAtm::Iterator it = atm_.find(ip);
			if (it == atm_.end()) {
				atm_.insert(ip, GMac::nullMac());
				bool res = atm_.wait();
				if (!res) {
					qWarning() << QString("can not resolve %1").arg(QString(ip));
					break;
				}
				it = atm_.find(ip);
				Q_ASSERT(it != atm_.end());
				qDebug() << QString("resolved %1 %2").arg(QString(it.key())).arg(QString(it.value()));
			}
			ethHdr->dmac_ = it.value();
		}

		if (it == cim_.end()) {
			GMac mac = packet.ethHdr_->smac();
			ci.mac_ = mac;
			ci.session_ = session;
			{
				QMutexLocker ml(&cim_.m_);
				it  = cim_.insert(mac, &ci);
			}
			qWarning() << QString("insert %1").arg(QString(mac));
		}
		GIp sip = ipHdr->sip();
		if (!sip.isNull())
			(*it)->ip_ = sip;

		GPacket::Result res = pcapDevice_.write(&packet);
		if (res != GPacket::Ok) {
			qWarning() << QString("pcapDevice_.write(&packet) return %d").arg(int(res));
		}
		qDebug() << QString("pcap write %1").arg(packet.buf_.size_);

	}
	if (it != cim_.end())
	{
		QMutexLocker ml(&cim_.m_);
		cim_.erase(it);
	}
	qDebug() << "";
}
