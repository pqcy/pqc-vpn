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

	bool start(int port) override;
	bool stop() override;

private:
	std::thread* acceptThread_{nullptr};
	void acceptRun();
	void _run(TcpSession* session);
};
