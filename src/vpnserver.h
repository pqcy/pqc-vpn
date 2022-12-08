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
	struct ClientInfoMapByIp : QMap<GIp, ClientInfo*> {
		QMutex m_;
	} cimbyip_;

	VpnServer(QObject* parent = nullptr);
	~VpnServer() override;

	QString intfName_;

protected:
	GSyncPcapDevice pcapDevice_{this};
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

	void run(Session* session) override;
};
typedef VpnServer *PVpnServer;
