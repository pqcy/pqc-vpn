#include <chrono>
#include <csignal>
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
		printf("syntax : vpnserver-test <port> <pem file name> <interface name>\n");
		printf("sample : vpnserver-test 12345 rootCA.pem eth0\n");
	}
};

VpnServer vs;
struct Obj : QObject {
	Q_OBJECT

public:
	Obj() : QObject(nullptr) {
		QObject::connect(&vs, &GStateObj::closed, this, &Obj::processClosed);
	}

public slots:
	void processClosed() {
		vs.close();
	}
} obj;

void signalHandler(int signo) {
	qWarning() << QString("signal occured %1").arg(signo);
	vs.close();
	qWarning() << "vpnserver terminated successfully";
	exit(0);
}

int main(int argc, char* argv[]) {
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	GApp a(argc, argv);

	Param param;
	if (!param.parse(argc, argv)) {
		Param::usage();
		return -1;
	}
	vs.port_ = param.port_;
#ifdef SUPPORT_VPN_TCP
#endif
#ifdef SUPPORT_VPN_TLS
	vs.pemFileName_ = param.pemFileName_;
#endif
#ifdef SUPPORT_VPN_PQC
	vs.pemFileName_ = param.pemFileName_;
#endif
	vs.intfName_ = param.intfName_.data();
	if (!vs.open()) {
		std::cerr << qPrintable(vs.err->msg()) << std::endl;
		return -1;
	}

	int res = a.exec();
	vs.close();
	return res;
}

#include "vpnserver-test.moc"
