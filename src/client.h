#pragma once

#include <GStateObj>
#include <GIp>
#include "session.h"

struct Client : GStateObj {
	Client(QObject* parent = nullptr);
	~Client() override;

	GIp localIp_{0};
	int localPort_{0};
	GIp ip_{0};
	int port_{0};
};
