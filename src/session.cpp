#include "session.h"

int Session::readAll(char* buf, int size) {
	int restSize = size;
	while (true) {
		int readLen = read(buf, restSize);
		if (readLen == -1)
			return -1;
		buf += readLen;
		restSize -= readLen;
		if (restSize == 0)
			return size;
	}
}
