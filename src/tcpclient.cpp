#include "tcpclient.h"

bool TcpClient::connect(GIp ip, int port) {
	sock_ = ::socket(AF_INET, SOCK_STREAM, 0);
	if (sock_ == -1) {
		error_ = strerror(errno);
        GTRACE("socket return -1 %s", error_.data());
		return false;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(ip);
	memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));

    int res = ::connect(sock_, (struct sockaddr *)&addr, sizeof(addr));
	if (res == -1) {
         error_ = strerror(errno);
         GTRACE("connect return -1 %s", error_.data());
		return false;
	}

	return true;
}
