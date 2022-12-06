#pragma once

#include "client.h"
#include "tcpsession.h"

struct TcpClient : public Client, public TcpSession {
	TcpClient(QObject* parent = nullptr);
	~TcpClient() override;

protected:
	bool doOpen() override;
	bool doClose() override;
};
