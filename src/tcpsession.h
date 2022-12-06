#pragma once

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>

#include "session.h"

#include "gtrace.h"

struct TcpSession : public Session {
	int sock_;

	TcpSession(int sock = 0);
	virtual ~TcpSession();

	int read(char* buf, int size) override;
	int write(char* buf, int size) override;
	bool close() override;
};
typedef TcpSession *PTcpSession;
