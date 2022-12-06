#pragma once

struct Session {
	virtual int read(char* buf, int size) = 0;
	virtual int write(char* buf, int size) = 0;
	virtual bool close() = 0;
};
typedef Session *PSession;
