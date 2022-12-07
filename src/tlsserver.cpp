#include "tlsserver.h"

#include <mutex>

TlsServer::TlsServer(QObject* parent) : TcpServer(parent) {
	TlsCommon::initialize();
}

TlsServer::~TlsServer() {
	close();
}

bool TlsServer::doOpen() {
	TlsCommon::initialize();

	const SSL_METHOD *method = TLS_server_method(); /* create new server-method instance */
	ctx_ = SSL_CTX_new(method); /* create new context from method */
	if ( ctx_ == NULL )
	{
		ERR_print_errors_fp(stderr);
		abort();
	}

	int res = SSL_CTX_use_certificate_file(ctx_, pemFileName_.data(), SSL_FILETYPE_PEM);
	if (res != 1) {
		SET_ERR(GErr::Fail, QString("SSL_CTX_use_certificate_file(%1) return %2").arg(pemFileName_.data()).arg(res));
		return false;
	}

	res = SSL_CTX_use_PrivateKey_file(ctx_, pemFileName_.data(), SSL_FILETYPE_PEM);
	if (res != 1) {
		SET_ERR(GErr::Fail, QString("SSL_CTX_use_PrivateKey_file(%1) return %2").arg(pemFileName_.data()).arg(res));
		return false;
	}

	res = SSL_CTX_check_private_key(ctx_);
	if (res != 1) {
		SET_ERR(GErr::Fail, QString("SSL_CTX_check_private_key return %1").arg(res));
		return false;
	}

	if (!bind())
		return false;

	acceptThread_ = new std::thread(&TlsServer::acceptRun, this);
	return true;
}

bool TlsServer::doClose() {
	::shutdown(acceptSock_, SHUT_RDWR);
	::close(acceptSock_);
	if (acceptThread_ != nullptr) {
		delete acceptThread_;
		acceptThread_ = nullptr;
	}

	sessions_.lock();
	for (TlsSession* session: sessions_)
		session->disconnect();
	sessions_.unlock();

	while (true) {
		sessions_.lock();
		bool exit = sessions_.size() == 0;
		sessions_.unlock();
		if (exit) break;
		usleep(100);
	}

	::SSL_CTX_free(ctx_);
	return true;
}

void TlsServer::acceptRun() {
	while (true) {
		struct sockaddr_in addr;
		socklen_t len = sizeof(addr);
		int newSock = ::accept(acceptSock_, (struct sockaddr *)&addr, &len);
		if (newSock == -1) {
			SET_ERR(GErr::Fail, QString("accept return -1 %1").arg(strerror(errno)));
			break;
		}
		SSL* ssl = SSL_new(ctx_);
		if (ssl == nullptr) {
			SET_ERR(GErr::Fail, "SSL_new return null");
			break;
		}

		int res = SSL_set_fd(ssl, newSock);
		if (res != 1) {
			SET_ERR(GErr::Fail, QString("SSL_set_fd return %1 %2").arg(res).arg(strerror(errno)));
			break;
		}

		res = SSL_accept(ssl);
		if (res == -1) {
			int sslError = SSL_get_error(ssl, res);
			qWarning() << QString("SSL_accept return -1 %1").arg(sslError);
			SSL_free(ssl);
			::shutdown(newSock, SHUT_RDWR);
			::close(newSock);
			continue;
		}

		TlsSession* session = new TlsSession(newSock, ssl);
		std::thread* thread = new std::thread(&TlsServer::_run, this, session);
		thread->detach();
	}
}

void TlsServer::_run(TlsSession* session) {
	sessions_.lock();
	sessions_.push_back(session);
	sessions_.unlock();

	run(session);

	sessions_.lock();
	sessions_.remove(session);
	sessions_.unlock();
}
