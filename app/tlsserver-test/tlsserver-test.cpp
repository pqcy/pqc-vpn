#include <chrono>
#include <iostream>
#include <thread>
#include <GApp>
#include <GThread>
#include "tlsserver.h"

struct ChatServer : public TlsServer {
protected:
	void run(Session* session) override {
		std::puts("connected");
		char buf[256];
		while (true) {
			int res = session->read(buf, 256);
			if (res <= 0) break;
			buf[res] = '\0';
			std::puts(buf);
			sessions_.lock();
			for (TlsSession* session: sessions_)
				session->write(buf, res);
			sessions_.unlock();
		}
		std::puts("disconnected");
	}
};

struct Param {
	int port_;
	std::string pemFileName_;

	bool parse(int argc, char** argv) {
		if (argc != 3) return false;
		port_ = std::stoi(argv[1]);
		pemFileName_ = argv[2];
		return true;
	}

	static void usage() {
		printf("syntax : tlsserver-test <port> <pem file name>\n");
		printf("sample : tlsserver-test 12345 rootCA.pem\n");
	}
};

int main(int argc, char* argv[]) {
	GApp a(argc, argv);

	ChatServer cs;

	Param param;
	if (!param.parse(argc, argv)) {
		Param::usage();
		return -1;
	}
	cs.port_ = param.port_;
	cs.pemFileName_ = param.pemFileName_;
	if (!cs.open()) {
		std::cerr << qPrintable(cs.err->msg()) << std::endl;
		return -1;
	}

	while (true) QThread::sleep(1);

	cs.close();
}
