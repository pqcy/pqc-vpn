#include "tlssession.h"
#include <QDebug>

TlsSession::TlsSession(int sock, SSL* ssl) {
	sock_ = sock;
	ssl_ = ssl;
}

TlsSession::~TlsSession() {
	TlsSession::disconnect();
}

int TlsSession::read(char* buf, int size) {
	int res = ::SSL_read(ssl_, buf, size);
	if (res == 0 || res == -1) {
		qWarning() << "SSL_read return" << res;
		return -1;
	}
	return res;
}

int TlsSession::write(char* buf, int size) {
	int res = ::SSL_write(ssl_, buf, size);
	if (res == 0 || res == -1) {
		qWarning() << "SSL_read return" << res;
		return -1;
	}
	return res;
}

bool TlsSession::disconnect() {
	if (ssl_ != nullptr) {
		::SSL_free(ssl_);
		ssl_ = nullptr;
	}
	if (sock_ != 0) {
		::shutdown(sock_, SHUT_RDWR);
		::close(sock_);
		sock_ = 0;
	}
	return true;
}
