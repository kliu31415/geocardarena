#include <string>
#include <SDL2/SDL_net.h>
#include "server.h"
#include "server_chat_box.h"
class LobbyServer
{
    enum class Flag{CHAT_BOX = 0};
    Server server;
    void process_data(Connection &c, std::string &data);
public:
    LobbyServer(Uint16 port);
    void operate();
};
