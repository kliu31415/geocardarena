#pragma once
#include <SDL2/SDL_net.h>
#include <vector>
#include <fstream>
#include <string>
#include <functional>
#include <set>
#include "connection.h"

void run_server();
extern std::set<std::string> user_names;
class Server
{
    constexpr static size_t MAX_CONNECTIONS = 200;
    TCPsocket server_socket;
    SDLNet_SocketSet socket_set;
    IPaddress IP;
    std::function<void(Connection&, std::string&)> process_data;
    std::vector<Connection> clients;
    std::ofstream log_fout;
    void log(std::string s);
public:
    Server() = default;
    void construct(Uint16 port, std::function<void(Connection&, std::string&)> process_data);
    ~Server();
    void operate();
    template<class Flag> void send_to_all_clients(const std::string &data, Flag flag)
    {
        for(auto &i: clients)
            i.send(data, flag);
    }
    Uint16 get_port() const;
};
