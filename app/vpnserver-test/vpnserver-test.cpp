#include <chrono>
#include <iostream>
#include <thread>

#include <GApp>

#include "vpnserver.h"

struct Param {
	int port_;
	std::string pemFileName_;
	std::string intfName_;

	bool parse(int argc, char** argv) {
		if (argc != 4) return false;
		port_ = std::stoi(argv[1]);
		pemFileName_ = argv[2];
		intfName_ = argv[3];
		return true;
	}

	static void usage() {
		printf("syntax : vpnserver-test <port> <pem file name> <interface>\n");
		printf("sample : vpnserver-test 12345 rootCA.pem eth0\n");
	}
};

int main(int argc, char* argv[]) {
	GApp a(argc, argv);

	VpnServer vs;

	Param param;
	if (!param.parse(argc, argv)) {
		Param::usage();
		return -1;
	}
	vs.port_ = param.port_;
	//vs.pemFileName_ = param.pemFileName_;
	vs.pcapDevice_.intfName_ = param.intfName_.data();
	if (!vs.open()) {
		std::cerr << qPrintable(vs.err->msg()) << std::endl;
		return -1;
	}

	while (true) QThread::sleep(1);

	vs.close();
}
