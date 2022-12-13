#pragma once

#include <QMutex>
#include <GAtm>
#include <GPcapDeviceWrite>
#include <GSyncPcapDevice>
#include "pqcserver.h"

#ifdef SUPPORT_VPN_TCP
struct VpnServer : TcpServer {
#endif
#ifdef SUPPORT_VPN_TLS
struct VpnServer : TlsServer {
#endif
#ifdef SUPPORT_VPN_PQC
	struct VpnServer : PqcServer {
#endif
	Q_OBJECT

public:
	struct ClientInfo {
		GMac mac_{GMac::nullMac()};
		GIp ip_{0};
		Session* session_{nullptr};
	};
	struct ClientInfoMap : QMap<GMac, ClientInfo*> {
		QMutex m_;
	} cim_;

	VpnServer(QObject* parent = nullptr);
	~VpnServer() override;

	QString intfName_;

protected:
	GSyncPcapDevice pcapDevice_{this};
	GSyncPcapDevice arpPcapDevice_{this};
	GAtm atm_{this};
	GIntf* intf_{nullptr};

protected:
	bool doOpen() override;
	bool doClose() override;

	struct CaptureAndProcessThread : GThread {
		CaptureAndProcessThread(QObject* parent) : GThread(parent) {}
		~CaptureAndProcessThread() override {}

		void run() override;
	} captureAndProcessThread_{this};

	struct ArpResolveThread : GThread {
		ArpResolveThread(QObject* parent) : GThread(parent) {}
		~ArpResolveThread() override {}

		void run() override;
	} arpResolveThread_{this};

	void run(Session* session) override;
};
typedef VpnServer *PVpnServer;
