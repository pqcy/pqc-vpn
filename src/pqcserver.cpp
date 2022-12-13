#include "pqcserver.h"

PqcServer::PqcServer(QObject* parent) : TlsServer(parent) {
	qDebug() << "";
}

PqcServer::~PqcServer() {
	qDebug() << "";
	close();
}

bool PqcServer::doOpen() {
	return TlsServer::doOpen();
}

bool PqcServer::doClose() {
	return TlsServer::doClose();
}
