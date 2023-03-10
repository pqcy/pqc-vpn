#pragma once

#include "tcpclient.h"
#include "tlssession.h"

struct TlsClient : public Client, public TlsSession {
	TlsClient(QObject* parent = nullptr);
	~TlsClient() override;

	TcpClient tcpClient_;
	SSL_CTX *ctx_{nullptr};

protected:
	virtual int configCtx();

protected:
	bool doOpen() override;
	bool doClose() override;
};
