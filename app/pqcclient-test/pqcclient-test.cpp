#include <iostream>
#include <thread>

#include <GApp>

#include "pqcclient.h"

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
		printf("syntax : pqcclient-test <ip> <port>\n");
		printf("sample : pqcclient-test 127.0.0.1 12345\n");
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

void inputAndSend(Session* session) {
	while (true) {
		std::string msg;
		std::getline(std::cin, msg);
		int writeLen = session->write(msg.data(), msg.size());
		if (writeLen == -1) break;
	}
}

int main(int argc, char* argv[]) {
	GApp a(argc, argv);

	PqcClient pc;

	Param param;
	if (!param.parse(argc, argv)) {
		Param::usage();
		return -1;
	}

	pc.ip_ = param.ip_;
	pc.port_ = param.port_;
	if (!pc.open()) {
		std::cerr << qPrintable(pc.err->msg()) << std::endl;
		return -1;
	}

	std::thread readAndPrintThread(&readAndPrint, &pc);
	std::thread inputAndSendThread(&inputAndSend, &pc);

	int res = a.exec();

	readAndPrintThread.join();
	inputAndSendThread.join();

	pc.close();
	return res;
}
