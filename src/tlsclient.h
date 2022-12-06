#pragma once

#include "tcpclient.h"
#include "tlssession.h"

struct TlsClient : public Client, public TlsSession {
	TcpClient tcpClient_;
	SSL_CTX *ctx_{nullptr};

    bool connect(GIp ip, int port) override;
};
