#include "client.h"
#include "fundamentals.h"
#include <future>
#include <chrono>
using namespace std;
const char * const DEFAULT_DOMAIN = "127.0.0.1";
Connection Client::connect_async(Uint16 port)
{
    disconnect();
    IPaddress IP;
    if(SDLNet_ResolveHost(&IP, DEFAULT_DOMAIN, port))
    {
        println("SDLNet_ResolveHost error: " + to_str(SDLNet_GetError()));
    }
    auto socket = SDLNet_TCP_Open(&IP);
    if(socket == nullptr)
    {
        println("SDLNet_TCP_Open error: " + to_str(SDLNet_GetError()));
        connection.construct(socket, IP);
    }
    else
    {
        println("successfully connected to server");
        connection.construct(socket, IP);
        SDLNet_TCP_AddSocket(socket_set, connection.socket);
    }
    return connection;
}
Client::Client()
{
    socket_set = SDLNet_AllocSocketSet(1);
    connection.socket = nullptr;
}
void Client::disconnect()
{
    if(connection.socket != nullptr)
    {
        SDLNet_TCP_DelSocket(socket_set, connection.socket);
        SDLNet_TCP_Close(connection.socket);
        connection.socket = nullptr;
    }
}
void Client::connect(Uint16 port, function<void(Connection&, string&)> process_data)
{
    if(!async_return.valid()) //not connected and not trying to connect, so attempt to connect
    {
        if(!connection.is_connected())
        {
            auto func = [=](Uint16 port) -> Connection {return Client::connect_async(port);};
            async_return = std::move(std::async(std::launch::async, func, port));
            this->process_data = process_data;
        }
    }
    else if(async_return.wait_for(chrono::milliseconds(0)) == future_status::ready) //connection function finished
    {
        connection = async_return.get();
    }
}
Client::~Client()
{
    println("closing client");
    disconnect();
    SDLNet_FreeSocketSet(socket_set);
}
void Client::operate()
{
    if(SDLNet_CheckSockets(socket_set, 0) > 0)
    {
        connection.receive(println, process_data);
    }
    if(connection.is_connected() && connection.operate(println, process_data))
    {
        SDLNet_TCP_DelSocket(socket_set, connection.socket);
        SDLNet_TCP_Close(connection.socket);
        connection.socket = nullptr;
    }
}
Uint16 Client::get_port() const
{
    return SDLNet_Read16(&connection.IP.port);
}
