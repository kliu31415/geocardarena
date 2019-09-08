#include <cstdlib>
#include <string>
#include <chrono>
#include <SDL2/SDL_image.h>
#include <iostream>
#include "sdl_base.h"
#include "menu.h"
#include "game_state.h"
#include "unit.h"
#include "spell.h"
#include "server.h"
#include "lobby_client.h"
#include "cpu_load_calculator.h"
using namespace std;
void run_client()
{
    println("starting program to run as client");
    initSDL("Geo Card Arena");
    StandardMenu::init();
    //init order may matter
    Card::init();
    Unit::init(true);
    Spell::init(true);
    init_misc_sprites();
    CPULoadCalculator cpu_load_calculator(1e9);
    SDL_Surface *window_icon = IMG_Load("data/resource/aether.png");
    SDL_SetColorKey(window_icon, SDL_TRUE, SDL_MapRGB(window_icon->format, 0, 0, 0));
    setWindowIcon(window_icon);
    lobby_client = std::make_shared<LobbyClient>();
    global_menu_stack = make_shared<std::stack<std::shared_ptr<Menu> > >();
    menuStack = global_menu_stack;
    menuStack->push(main_menu);
    while(true)
    {
        cpu_load_calculator.begin_counting();
        switch(current_global_game_state)
        {
        case global_game_state::menu:
            menuStack->top()->draw();
            menuStack->top()->operate();
            break;
        case global_game_state::online_lobby:
            lobby_client->render();
            lobby_client->operate();
            break;
        case global_game_state::in_game:
            game_state->render();
            game_state->operate();
            break;
        }
        cpu_load_calculator.end_counting();
        drawText(to_str(round(cpu_load_calculator.get_load(), 2)), 0, 0, getFontSize(0), 150, 100, 100, 150);
        updateScreen();
    }
}
int main(int argc, char **argv)
{
    if(argc==2 && argv[1] == (string)"server")
    {
        thread server_thread(run_server);
        string command;
        while(true)
        {
            getline(cin, command);
            if(command == "quit")
                exit(0);
            else if(command == "uptime")
                println(seconds_to_str(getTicksS()));
        }
    }
    else run_client();
}
