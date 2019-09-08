#include "lobby_client.h"
#include "sdl_base.h"
#include "fundamentals.h"
#include <cstdlib>
#include <functional>
using namespace std;
void LobbyClient::send_chat_func(string &data)
{
    client.connection.send(data, Flag::CHAT_BOX);
    chat_box.add_chat_line(data);
}
void LobbyClient::process_data(Connection &connection, string &data)
{
    if(data.empty())
        return;
    switch(Connection::poll_data_flag<Flag>(data))
    {
    case Flag::CHAT_BOX: //chat message received from server
        chat_box.add_chat_line(data);
        break;
    }
}
void LobbyClient::connect()
{
    client.connect(LOBBY_PORT, [&](Connection &c, string &data)-> void {process_data(c, data);});
}
LobbyClient::LobbyClient()
{
    function<int()> y_func = []() -> int {return getWindowH() * 0.75;};
    function<int()> w_func = []() -> int {return getWindowW();};
    function<int()> h_func = []() -> int {return getWindowH() * 0.25 - getFontH(getFontSize(0) * 0.9);};
    function<int()> fsz_func = []() -> int {return getFontSize(0) * 0.9;};
    chat_box = ClientChatBox(0, y_func, w_func, h_func, fsz_func, 250, 250, 250);
}
void LobbyClient::operate()
{
    if(client.connection.is_connected())
    {
        client.operate();
    }
    else
    {
        connect();
    }
    SDL_Event input;
    while(SDL_PollEvent(&input))
    {
        switch(input.type)
        {
        case SDL_QUIT:
            exit(EXIT_SUCCESS);
            break;
        case SDL_MOUSEWHEEL:
            break;
        case SDL_KEYDOWN:
            break;
        }
        chat_box.operate([&](string &data) -> void {send_chat_func(data);}, input);
    }
}
void LobbyClient::render()
{
    renderClear(230, 230, 230);
    VerticalTextDrawer text_drawer(0, getWindowH() * 0.8 - 2 * getFontH(getFontSize(0)), getFontSize(0));
    if(client.connection.is_connected())
    {
        text_drawer.draw("connected");
        text_drawer.draw(to_str(client.connection.get_ping()));
    }
    else
    {
        text_drawer.draw("disconnected");
    }
    chat_box.render();
}
