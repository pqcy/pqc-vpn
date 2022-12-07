#pragma once

#include <list>
#include <mutex>
#include <thread>

#include "tcpserver.h"
#include "tlssession.h"

struct TlsServer : public TcpServer {
	TlsServer(QObject* parent = nullptr);
	~TlsServer() override;

	SSL_CTX *ctx_;

	struct TlsSessionList : std::list<TlsSession*> {
	protected:
		std::mutex m_;
	public:
		void lock() { m_.lock(); }
		void unlock() { m_.unlock(); }
	} sessions_;


	std::string pemFileName_;

protected:
	bool doOpen() override;
	bool doClose() override;

private:
	std::thread* acceptThread_{nullptr};
	void acceptRun();
	void _run(TlsSession* session);
};
