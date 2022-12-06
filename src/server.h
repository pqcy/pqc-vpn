#pragma once

#include <string>

#include "session.h"

struct Server {
	std::string error_;

	virtual bool start(int port) = 0;
	virtual bool stop() = 0;

protected:
	virtual void run(Session* session) = 0;
};
