#include <iostream>
#include <thread>

#include <GApp>

#include "vpnclient.h"

struct Param {
	QString dummyIntfName_;
	QString realIntfName_;
	GIp ip_;
	int port_;

	bool parse(int argc, char** argv) {
		if (argc != 4) return false;
		dummyIntfName_ = argv[1];
		realIntfName_ = argv[2];
		ip_ = GIp(argv[3]);
		port_ = std::stoi(argv[4]);
		return true;
	}

	static void usage() {
		printf("syntax : vpnclient-test <dummy interface> <real interface> <ip> <port>\n");
		printf("sample : vpnclient-test dum0 wlan0 127.0.0.1 12345\n");
	}
};

int main(int argc, char* argv[]) {
	GApp a(argc, argv);

	VpnClient vc;

	Param param;
	if (!param.parse(argc, argv)) {
		Param::usage();
		return -1;
	}

	vc.dummyPcapDevice_.intfName_ = param.dummyIntfName_;
	vc.tcpClient_.ip_ = param.ip_;
	vc.tcpClient_.port_ = param.port_;

	if (!vc.open()) {
		std::cerr << qPrintable(vc.err->msg()) << std::endl;
		return -1;
	}

	while (vc.active())
		QThread::sleep(1);

	vc.close();
}
