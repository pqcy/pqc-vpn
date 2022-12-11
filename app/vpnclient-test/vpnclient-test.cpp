#include <csignal>
#include <iostream>
#include <thread>

#include <GApp>

#include "vpnclient.h"

struct Param {
	QString dummyIntfName_;
	QString dummyIntfMac_;
	QString realIntfName_;
	GIp serverIp_;
	int serverPort_;

	bool parse(int argc, char** argv) {
		if (argc != 6) return false;
		dummyIntfName_ = argv[1];
		dummyIntfMac_ = argv[2];
		realIntfName_ = argv[3];
		serverIp_ = GIp(argv[4]);
		serverPort_ = std::stoi(argv[5]);
		return true;
	}

	static void usage() {
		printf("syntax : vpnclient-test <dummy interface name> <dummy interface mac> <real interface name> <server ip> <server port>\n");
		printf("sample : vpnclient-test dum0 00:00:00:11:11:11 wlan0 127.0.0.1 12345\n");
	}
};

VpnClient vc;
struct Obj : QObject {
	Q_OBJECT

public:
	Obj() : QObject(nullptr) {
		QMetaObject::Connection conn = QObject::connect(&vc, &GStateObj::closed, this, &Obj::processClosed);
		qDebug() << conn;
	}

public slots:
	void processClosed() {
		GApp::quit();
	}
} obj;

void signalHandler(int signo) {
	qWarning() << QString("signal occured %1").arg(signo);
	vc.close();
	qWarning() << "vpnclient terminated successfully";
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

	vc.dummyIntfName_ = param.dummyIntfName_;
	vc.dummyIntfMac_ = param.dummyIntfMac_;
	vc.realIntfName_ = param.realIntfName_;
	vc.serverIp_ = param.serverIp_;
	vc.serverPort_ = param.serverPort_;
	if (!vc.open()) {
		std::cerr << qPrintable(vc.err->msg()) << std::endl;
		return -1;
	}

	int res = a.exec();
	vc.close();
	return res;
}

#include "vpnclient-test.moc"
