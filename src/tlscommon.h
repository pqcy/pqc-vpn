#pragma once

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>

// #define SUPPORT_VPN_TLS

struct TlsCommon {
	static void initialize();
};
