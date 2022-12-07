#pragma once

#include <GStateObj>
#include "session.h"

struct Server : GStateObj {
	Server(QObject* parent = nullptr);
	~Server() override;

	int port_;

protected:
	virtual void run(Session* session) = 0;
};
