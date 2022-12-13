#pragma once

#include <GSyncPcapDevice>
#include <GRawIpSocketWrite>
#include "tlsclient.h"

struct VpnClient : GStateObj {
	Q_OBJECT

public:
	VpnClient(QObject* parent = nullptr);
	~VpnClient() override;

	QString dummyIntfName_;
	QString dummyIntfMac_;
	QString realIntfName_;
	QString intfName_;
	GIp serverIp_;
	int serverPort_;

protected:
#ifdef SUPPORT_VPN_TCP
	TcpClient sockClient_{this};
#endif
#ifdef SUPPORT_VPN_TLS
	TlsClient sockClient_{this};
#endif
#ifdef SUPPORT_VPN_PQC
	PqcClient sockClient_{this};
#endif
	GSyncPcapDevice dummyPcapDevice_{this};
	GRawIpSocketWrite socketWrite_{this};

protected:
	bool doOpen() override;
	bool doClose() override;

	static void runCommand(QString program, bool sync = true);
	void dhcpAndAddRouteTable();

	struct CaptureAndSendThread : GThread {
		CaptureAndSendThread(QObject* parent) : GThread(parent) {}
		~CaptureAndSendThread() override {}

		void run() override;
	} captureAndSendThread_{this};

	struct ReadAndReplyThread : GThread {
		ReadAndReplyThread(QObject* parent) : GThread(parent) {}
		~ReadAndReplyThread() override {}

		void run() override;
	} readAndReplyThread_{this};
};
typedef VpnClient *PVpnClient;
