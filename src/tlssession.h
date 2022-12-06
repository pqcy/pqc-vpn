#pragma once

#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "session.h"

struct TlsSession : public Session {
	int sock_;
	SSL* ssl_;

	TlsSession(int sock = 0, SSL* ssl = 0);
	virtual ~TlsSession();

	int read(char* buf, int size) override;
	int write(char* buf, int size) override;
	bool disconnect() override;
};
typedef TlsSession *PTlsSession;
