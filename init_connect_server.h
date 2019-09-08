#pragma once
#include <string>
#include <SDL2/SDL_net.h>
#include "server.h"
class InitConnectServer
{
    Server server;
    void process_data(Connection &c, std::string &data);
public:
    enum class Flag{USER_INFO};
    InitConnectServer(Uint16 port);
    void operate();
};
