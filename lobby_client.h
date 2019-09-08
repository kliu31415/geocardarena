#pragma once
#include <SDL2/SDL_net.h>
#include <string>
#include <vector>
#include "client_chat_box.h"
#include "client.h"
#include "lobby_gamelist.h"

class LobbyClient
{
    enum class Flag{CHAT_BOX = 0};
    ClientChatBox chat_box;
    Client client;
    void send_chat_func(std::string &data);
    void process_data(Connection &c, std::string &data);
public:
    LobbyClient();
    void connect();
    void operate();
    void render();
};
