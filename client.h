#pragma once
#include <string>
#include <functional>
#include <cstdint>
#include <future>
#include <utility>
#include <SDL2/SDL_net.h>
#include "connection.h"
class Client
{
    SDLNet_SocketSet socket_set;
    std::function<void(Connection&, std::string&)> process_data;
    std::future<Connection> async_return;
    Connection connect_async(Uint16 port);
public:
    Connection connection;
    Client();
    void disconnect();
    void connect(Uint16 port, std::function<void(Connection&, std::string&)> process_data);
    ~Client();
    void operate();
    Uint16 get_port() const;
};
