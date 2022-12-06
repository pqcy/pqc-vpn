#include "tcpsession.h"

TcpSession::TcpSession(int sock) {
	sock_ = sock;
}

TcpSession::~TcpSession() {
	close();
}

int TcpSession::read(char* buf, int size) {
	ssize_t res = ::recv(sock_, buf, size, 0);
	if (res == 0 || res == -1) {
		return -1;
	}
	return res;
}

int TcpSession::write(char* buf, int size) {
	ssize_t res = ::send(sock_, buf, size, 0);
	if (res == 0 || res == -1) {
		return -1;
	}
	return res;
}

bool TcpSession::close() {
	if (sock_ != 0) {
		::shutdown(sock_, SHUT_RDWR);
		::close(sock_);
		sock_ = 0;
	}
	return true;
}
