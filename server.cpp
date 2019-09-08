#include "server.h"
#include "fundamentals.h"
#include "sdl_base.h"
#include <SDL2/SDL_net.h>
#include <string>
#include <thread>
#include <iostream>
#include <cstdlib>
#include <set>
using namespace std;
set<string> user_names;
void Server::log(string s)
{
    log_fout << "[" << seconds_to_str(getTicksS()) << "]" << s << "\n";
    cout << "[" << seconds_to_str(getTicksS()) << "]" << s << "\n";
}
void Server::construct(Uint16 port, function<void(Connection&, string&)> process_data)
{
    log("starting server");
    //create log file
    long long time_since_epoch = chrono::seconds(chrono::system_clock::now().time_since_epoch().count()).count();
    system("mkdir server_logs");
    log_fout.open(("server_logs/log" + to_str(time_since_epoch) + ".txt").c_str());
    if(log_fout.fail())
        cout << "failed to create server log file" << endl;
    //
    socket_set = SDLNet_AllocSocketSet(MAX_CONNECTIONS + 1);
    if(SDLNet_ResolveHost(&IP, nullptr, port))
        log("SDLNet_ResolveHost error: " + to_str(SDLNet_GetError()));
    server_socket = SDLNet_TCP_Open(&IP);
    SDLNet_TCP_AddSocket(socket_set, server_socket);
    //
    this->process_data = process_data;
}
Server::~Server()
{
    log("closing server");
    for(auto &i: clients)
    {
        SDLNet_TCP_DelSocket(socket_set, i.socket);
        SDLNet_TCP_Close(i.socket);
        user_names.erase(i.name);
    }
    SDLNet_FreeSocketSet(socket_set);
    log_fout.close();
}
void Server::operate()
{
    while(true)
    {
        TCPsocket new_client_socket = SDLNet_TCP_Accept(server_socket);
        if(new_client_socket != nullptr)
        {
            IPaddress *IP;
            IP = SDLNet_TCP_GetPeerAddress(new_client_socket);
            log("New connection accepted from " + IPv4_to_str(*IP));
            size_t num_active = SDLNet_TCP_AddSocket(socket_set, new_client_socket) - 1;
            if(num_active == 1)
                log("1 connection active");
            else log(to_str(num_active) + " connections active");
            clients.emplace_back();
            clients.back().construct(new_client_socket, *IP);
            //we don't need to free IP
        }
        else break;
    }
    if(SDLNet_CheckSockets(socket_set, 0) > 0)
    {
        for(auto &i: clients)
            i.receive([&](string s)-> void {Server::log(s);}, process_data);
    }
    for(auto i = clients.begin(); i!=clients.end(); )
    {
        if(i->operate([&](string s)-> void {Server::log(s);}, process_data))
        {
            SDLNet_TCP_DelSocket(socket_set, i->socket);
            SDLNet_TCP_Close(i->socket);
            user_names.erase(i->name);
            i = clients.erase(i);
        }
        else i++;
    }
}
Uint16 Server::get_port() const
{
    return SDLNet_Read16(&IP.port);
}
