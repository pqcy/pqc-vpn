#pragma once

#include <QMutex>
#include <GAtm>
#include <GPcapDeviceWrite>
#include <GSyncPcapDevice>
#include "tcpserver.h"

struct VpnServer : TcpServer {
	static const int MaxBufSize = 16384;

	struct ClientInfo {
		GMac mac_;
		GIp ip_;
		Session* session_;
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