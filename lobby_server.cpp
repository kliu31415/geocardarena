#include "lobby_server.h"
#include "sdl_base.h"
#include <thread>
#include <algorithm>
#include <cstdlib>
using namespace std;
void LobbyServer::process_data(Connection &connection, string &data)
{
    if(data.empty())
        return;
    switch(Connection::poll_data_flag<Flag>(data))
    {
    case Flag::CHAT_BOX: //received chat from one client.
        if(!data.empty() && data[0] == '/') //it's a chat command, so process it
        {
            data.erase(data.begin());
            if(data == "uptime")
            {
                connection.send("server uptime: " + seconds_to_str(getTicksS()), Flag::CHAT_BOX);
            }
            else if(data == "fkill")
            {
                println("received fkill command");
                exit(EXIT_SUCCESS);
            }
            else if(data == "ping")
            {
                connection.send("server ping: " + to_str(connection.get_ping()), Flag::CHAT_BOX);
            }
            else if(data == "help")
            {
                connection.send("figure out the commands by yourself :)", Flag::CHAT_BOX);
            }
            else if(data.size()>=4 && data.substr(0, 4) == "sudo")
            {
                connection.send("nice try, but nope", Flag::CHAT_BOX);
            }
            else
            {
                connection.send("unknown command: " + data, Flag::CHAT_BOX);
            }
        }
        else //send the chat to all clients connected to the lobby.
        {
            data = IPv4_to_str_no_port(connection.IP) + ":" + data; //add info on who sent it
            server.send_to_all_clients(data, Flag::CHAT_BOX);
        }
        break;
    }
}
LobbyServer::LobbyServer(Uint16 port)
{
    server.construct(port, [&](Connection &c, string &data)-> void {process_data(c, data);});
}
void LobbyServer::operate()
{
    server.operate();
}
#include "init_connect_server.h"
void run_server()
{
    println("starting program to run as server");
    atexit(SDLNet_Quit);
    SDLNet_Init();
    LobbyServer lobby_server(LOBBY_PORT);
    InitConnectServer init_connect_server(INIT_CONNECT_PORT);
    while(true)
    {
        unsigned rn = getTicks();
        lobby_server.operate();
        init_connect_server.operate();
        this_thread::sleep_for(chrono::milliseconds(max(10 - (int)(getTicks() - rn), 0)));
    }
}
