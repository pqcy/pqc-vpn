#pragma once

#include <GSyncPcapDevice>
#include <GRawIpSocketWrite>
#include "tlsclient.h"

struct VpnClient : GStateObj {
	Q_OBJECT

public:
	static const int MaxBufSize = 16384;

	VpnClient(QObject* parent = nullptr);
	~VpnClient() override;

	QString dummyIntfName_;
	QString realIntfName_;
	QString intfName_;
	GIp ip_;
	int port_;

protected:
#ifdef SUPPORT_VPN_TLS
	TlsClient sockClient_{this};
#else // SUPPORT_VPN_TLS
	TcpClient sockClient_{this};
#endif // SUPPORT_VPN_TLS
	GSyncPcapDevice dummyPcapDevice_{this};
	GRawIpSocketWrite socketWrite_{this};

protected:
	bool doOpen() override;
	bool doClose() override;

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
