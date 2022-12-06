#pragma once

#include <GStateObj>
#include "session.h"

struct Server : GStateObj {
	int port_;

protected:
	virtual void run(Session* session) = 0;
};
