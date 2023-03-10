#pragma once

#include "tlsclient.h"

struct PqcClient : public TlsClient {
	PqcClient(QObject* parent = nullptr);
	~PqcClient() override;

protected:
	int configCtx() override;

protected:
	bool doOpen() override;
	bool doClose() override;
};
