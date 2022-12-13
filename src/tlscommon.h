#pragma once

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>

// #define SUPPORT_VPN_TCP
// #define SUPPORT_VPN_TLS
#define SUPPORT_VPN_PQC

struct TlsCommon {
	static const int MaxBufSize = 65536;
	static void initialize();
};
