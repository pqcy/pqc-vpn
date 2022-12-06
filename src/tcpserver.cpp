#include "tcpserver.h"

#include <mutex>

bool TcpServer::start(int port) {
    acceptSock_ = ::socket(AF_INET, SOCK_STREAM, 0);
	if (acceptSock_ == -1) {
		error_ = strerror(errno);
        GTRACE("socket return -1 %s", error_.data());
		return false;
	}

	int res;
#ifdef __linux__
	int optval = 1;
	res = ::setsockopt(acceptSock_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (res == -1) {
		error_ = strerror(errno);
        GTRACE("setsockopt return -1 %s", error_.data());
		return false;
	}
#endif // __linux

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	ssize_t res2 = ::bind(acceptSock_, (struct sockaddr *)&addr, sizeof(addr));
    if (res2 == -1) {
		error_ = strerror(errno);
        GTRACE("bind return -1 %s", error_.data());
        return false;
	}

    res = ::listen(acceptSock_, 5);
	if (res == -1) {
		error_ = strerror(errno);
        GTRACE("listen return -1 %s", error_.data());
		return false;
	}

	acceptThread_ = new std::thread(&TcpServer::acceptRun, this);
	return true;
}

bool TcpServer::stop() {
    ::shutdown(acceptSock_, SHUT_RDWR);
	::close(acceptSock_);
	if (acceptThread_ != nullptr) {
		delete acceptThread_;
		acceptThread_ = nullptr;
	}

	sessions_.lock();
	for (TcpSession* session: sessions_)
		session->close();
	sessions_.unlock();

	while (true) {
		sessions_.lock();
		bool exit = sessions_.size() == 0;
		sessions_.unlock();
		if (exit) break;
		usleep(1000);
	}

	return true;
}

void TcpServer::acceptRun() {
	while (true) {
		struct sockaddr_in addr;
		socklen_t len = sizeof(addr);
		int newSock = ::accept(acceptSock_, (struct sockaddr *)&addr, &len);
		if (newSock == -1) {
			error_ = strerror(errno);
            GTRACE("listen return -1 %s", error_.data());
			break;
		}
		TcpSession* session = new TcpSession(newSock);
		std::thread* thread = new std::thread(&TcpServer::_run, this, session);
		thread->detach();
	}
}

void TcpServer::_run(TcpSession* session) {
    sessions_.lock();
    sessions_.push_back(session);
    sessions_.unlock();

	run(session);

	sessions_.lock();
	sessions_.remove(session);
	sessions_.unlock();
}
