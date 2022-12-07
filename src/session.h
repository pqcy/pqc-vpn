#pragma once

struct Session {
	virtual int read(char* buf, int size) = 0;
	virtual int write(char* buf, int size) = 0;
	virtual bool disconnect() = 0;

	int readAll(char* buf, int size);
};
typedef Session *PSession;
