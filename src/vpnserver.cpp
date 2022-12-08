#include "vpnserver.h"

VpnServer::VpnServer(QObject* parent) : TcpServer(parent) {
}

VpnServer::~VpnServer() {
	close();
}

bool VpnServer::doOpen() {
	if (!TcpServer::doOpen()) return false;

	pcapDevice_.intfName_ = intfName_;
	if (!pcapDevice_.open()) {
		err = pcapDevice_.err;
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

	return true;
}

bool VpnServer::doClose() {
	TcpServer::doClose();
	pcapDevice_.close();
	atm_.close();
	captureAndProcessThread_.quit();
	captureAndProcessThread_.wait();
	return true;
}

void VpnServer::CaptureAndProcessThread::run() {
	VpnServer* server = PVpnServer(parent());
	GSyncPcapDevice* pcapDevice = &server->pcapDevice_;
	ClientInfoMap* cim = &server->cim_;
	GIntf* intf = server->intf_;

	while (true) {
		GEthPacket packet;
		GPacket::Result res = pcapDevice->read(&packet);
		if (res == GPacket::Eof || res == GPacket::Fail) break;
		if (res == GPacket::None) continue;

		GEthHdr* ethHdr = packet.ethHdr_;
		if (ethHdr == nullptr) continue;

		GIpHdr* ipHdr = packet.ipHdr_;
		if (ipHdr == nullptr) continue;

		GMac smac = ethHdr->smac();
		GMac dmac = ethHdr->dmac();
		{
			QMutexLocker ml(&cim->m_);
			if (cim->find(smac) != cim->end()) continue; // client sending packet
		}
		uint16_t len = sizeof(GEthHdr) + ipHdr->len();
		qDebug() << QString("len=%1 smac=%2 dmac=%3").arg(len).arg(QString(smac)).arg(QString(dmac));

		char buf[MaxBufSize];
		memcpy(buf, "PQ", 2);
		*reinterpret_cast<uint16_t*>(&buf[2]) = htons(len);
		memcpy(&buf[4], packet.buf_.data_, len);

		if (dmac.isBroadcast() || dmac.isMulticast()) {
			qWarning() << "broadcast or multicast"; // gilgil temp 2022.12.08
			QMutexLocker ml(&cim->m_);
			for (ClientInfo* ci : *cim) {
				ci->session_->write(buf, 4 + len);
				qWarning() << QString("session write %1 %2").arg(4 + len).arg(QString(ci->mac_));
			}
		} else {
			// ----- by gilgil 2022.-----
			// Even if packet is for a specific client(e.g. gateway > client), dmac can be server's real mac
			if (dmac == intf->mac()) {

			}
			// ---------------------
			ClientInfoMap::iterator it = cim->find(dmac);
			if (it != cim->end()) {
				ClientInfo* ci = *it;
				Session* session = ci->session_;
				int writeLen = session->write(buf, 4 + len);
				if (writeLen == -1) break;
				qWarning() << QString("session write %1 %2").arg(4 + len).arg(QString(ci->mac_));
			}
		}
	}
}

void VpnServer::run(Session* session) {
	qDebug() << "";
	bool ciAdded = false;
	ClientInfo ci;

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
		readLen = session->readAll(buf, len);
		if (readLen != len) {
			qWarning() << QString("readLen=%1 len=%2").arg(readLen).arg(len);
			break;
		}

		GEthPacket packet;
		packet.buf_.data_ = pbyte(buf);
		packet.buf_.size_ = len;
		packet.parse();

		GEthHdr* ethHdr = packet.ethHdr_;
		if (ethHdr == nullptr) continue;

		GMac smac = ethHdr->smac();
		GMac dmac = ethHdr->dmac();
		if (smac == dmac) {
			GIpHdr* ipHdr = packet.ipHdr_;
			if (ipHdr == nullptr) continue;
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

		if (!ciAdded) {
			GMac mac = packet.ethHdr_->smac();
			ci.mac_ = mac;
			ci.session_ = session;
			{
				QMutexLocker ml(&cim_.m_);
				cim_.insert(mac, &ci);
			}
			qWarning() << QString("insert %1").arg(QString(mac));
			ciAdded = true;
		}
		GPacket::Result res = pcapDevice_.write(&packet);
		if (res != GPacket::Ok) {
			qWarning() << QString("pcapDevice_.write(&packet) return %d").arg(int(res));
		}
		qWarning() << QString("pcap write %1").arg(packet.buf_.size_);

	}
	if (ciAdded)
	{
		QMutexLocker ml(&cim_.m_);
		cim_.remove(ci.mac_);
		cimbyip_.remove(ci.ip_);
	}
	qDebug() << "";
}
