#include "client.h"

Client::Client(QObject* parent) : GStateObj(parent) {
}

Client::~Client() {
	close();
}
