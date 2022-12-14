#include "pqcclient.h"

PqcClient::PqcClient(QObject* parent) : TlsClient(parent) {
	qDebug() << "";
}

PqcClient::~PqcClient() {
	qDebug() << "";
	close();
}

int PqcClient::configCtx() {
	STACK_OF(OPENSSL_STRING) *ssl_args = sk_OPENSSL_STRING_new_null();; //groups flag, kem alg id
	std::string groups = "-groups"; //groups flag
	std::string alg = "kyber512";   //PQC Algorithm name
	SSL_CONF_CTX *cctx = SSL_CONF_CTX_new();

	SSL_CONF_CTX_set_flags(cctx, SSL_CONF_FLAG_CLIENT | SSL_CONF_FLAG_CMDLINE);
	SSL_CONF_CTX_set_ssl_ctx(cctx, ctx_);

	/*for SSL_CONF_CTX flag set*/
	if (!sk_OPENSSL_STRING_push(ssl_args, groups.data()) || !sk_OPENSSL_STRING_push(ssl_args, alg.data())){
		printf("[Error] Memory allcotated\nPlease try again.\n");
		return false;
	}
	for (int i = 0; i < sk_OPENSSL_STRING_num(ssl_args); i += 2) {
		const char *flag = sk_OPENSSL_STRING_value(ssl_args, i);
		const char *arg = sk_OPENSSL_STRING_value(ssl_args, i + 1);
		if(SSL_CONF_cmd(cctx, flag, arg) <= 0){
			if(arg != NULL){
				printf("[Algorithms Set] flag: \"%s\", Algorithm name: \"%s\"\n", flag, arg);
				printf("[NOTICE] Did you export LD_LIBRARY_PATH=pqc-app/lib \?\n");
				printf("[NOTICE] Please Check a Directory name is pqc-app/lib\n");
			}
			else{
				printf("[Error] Fail to set a PQC_Algorithm.\n");
			}
			printf("[NOTICE] Current TLS is no PQC\n");
			return 1;
		}
	}
	if(!SSL_CONF_CTX_finish(cctx)){
		printf("[Error] Can't finishing context\n");
		return 0;
	}
	return 1;
}

bool PqcClient::doOpen() {
	return TlsClient::doOpen();
}

bool PqcClient::doClose() {
	return TlsClient::doClose();
}
