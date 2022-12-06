#pragma once

#include <GStateObj>
#include <GIp>
#include "session.h"

struct Client : GStateObj {
	Client(QObject* parent = nullptr);
	~Client() override;

	GIp ip_;
	int port_;
};
