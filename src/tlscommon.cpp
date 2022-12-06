#include "tlscommon.h"

#include <openssl/ssl.h>

void TlsCommon::initialize() {
    static bool first = true;
    if (first) {
        OpenSSL_add_all_algorithms(); /* Load cryptos, et.al. */
        SSL_load_error_strings(); /* Bring in and register error messages */
        first = false;
    }
}
