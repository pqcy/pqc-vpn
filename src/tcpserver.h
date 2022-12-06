#pragma once

#include <list>
#include <mutex>
#include <thread>

#include "server.h"
#include "tcpsession.h"

struct TcpServer : public Server {
	int acceptSock_;

	struct TcpSessionList : std::list<TcpSession*> {
	protected:
		std::mutex m_;
	public:
		void lock() { m_.lock(); }
		void unlock() { m_.unlock(); }
	} sessions_;

    int port_;

protected:
    bool bind();
    bool doOpen() override;
    bool doClose() override;

private:
	std::thread* acceptThread_{nullptr};
	void acceptRun();
	void _run(TcpSession* session);
};
