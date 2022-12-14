#include "pqcclient.h"

PqcClient::PqcClient(QObject* parent) : TlsClient(parent) {
	qDebug() << "";
}

PqcClient::~PqcClient() {
	qDebug() << "";
	close();
}

int PqcClient::configCtx() {

}

bool PqcClient::doOpen() {
	return TlsClient::doOpen();
}

bool PqcClient::doClose() {
	return TlsClient::doClose();
}
