#include "init_connect_server.h"
#include "sdl_base.h"
#include <thread>
#include <algorithm>
#include <cstdlib>
using namespace std;
void InitConnectServer::process_data(Connection &connection, string &data)
{
    if(data.empty())
        return;
    switch(Connection::poll_data_flag<Flag>(data))
    {
    case Flag::USER_INFO:
        if(data.size() < 3 || data.size() > 15)
        {

        }
        else if(user_names.count(data))
        {

        }
        else
        {

        }
        break;
    }
}
InitConnectServer::InitConnectServer(Uint16 port)
{
    server.construct(port, [&](Connection &c, string &data)-> void {process_data(c, data);});
}
void InitConnectServer::operate()
{
    server.operate();
}
