#pragma once

#include <list>
#include <mutex>
#include <thread>

#include "server.h"
#include "tlssession.h"

struct TlsServer : public Server {
    int acceptSock_;
    SSL_CTX *ctx_;

    struct TlsSessionList : std::list<TlsSession*> {
    protected:
        std::mutex m_;
    public:
        void lock() { m_.lock(); }
        void unlock() { m_.unlock(); }
    } sessions_;

    std::string pemFileName_;
    bool start(int port) override;
    bool stop() override;

private:
    std::thread* acceptThread_{nullptr};
    void acceptRun();
    void _run(TlsSession* session);
};
