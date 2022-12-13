#pragma once

#include "tlsserver.h"

struct PqcServer : public TlsServer {
	PqcServer(QObject* parent = nullptr);
	~PqcServer() override;

protected:
	bool doOpen() override;
	bool doClose() override;
};
