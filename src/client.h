#pragma once

#include <GIp>

struct Client {
    std::string error_;

    virtual bool connect(GIp ip, int port) = 0;
};
