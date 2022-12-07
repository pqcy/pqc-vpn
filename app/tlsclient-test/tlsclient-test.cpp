#include <iostream>
#include <thread>

#include <GApp>

#include "tlsclient.h"

struct Param {
	GIp ip_;
	int port_;

	bool parse(int argc, char** argv) {
		if (argc != 3) return false;
		ip_ = GIp(argv[1]);
		port_ = std::stoi(argv[2]);
		return true;
	}

	static void usage() {
		printf("syntax : tlsclient-test <ip> <port>\n");
		printf("sample : tlsclient-test 127.0.0.1 12345\n");
	}
};

void readAndPrint(Session* session) {
	std::puts("connected");
	char buf[256];
	while (true) {
		int res = session->read(buf, 256);
		if (res <= 0) break;
		buf[res] = '\0';
		std::puts(buf);
	}
	std::puts("disconnected");
	exit(0);
}

int main(int argc, char* argv[]) {
	GApp a(argc, argv);

	TlsClient tc;

	Param param;
	if (!param.parse(argc, argv)) {
		Param::usage();
		return -1;
	}

	tc.ip_ = param.ip_;
	tc.port_ = param.port_;
	if (!tc.open()) {
		std::cerr << qPrintable(tc.err->msg()) << std::endl;
		return -1;
	}

	std::thread thread(&readAndPrint, &tc);

	while (true) {
		std::string msg;
		std::getline(std::cin, msg);
		int writeLen = tc.write(msg.data(), msg.size());
		if (writeLen == -1) break;
	}

	tc.close();
}
