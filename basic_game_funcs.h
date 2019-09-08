#pragma once
#include "sdl_base.h"
#include <functional>
#include <memory>
#include <stack>
#define GAME_VERSION_MAJOR 0
#define GAME_VERSION_MINOR 0
#define GAME_VERSION_PATCH 0
#define GAME_VERSION_DESC "a"
extern bool SHOW_EXTRA_INFO;
class Menu;
extern std::shared_ptr<std::stack<std::shared_ptr<Menu> > > global_menu_stack;
enum class global_game_state{menu, in_game, online_lobby};
extern global_game_state current_global_game_state;
class GameState;
extern std::shared_ptr<GameState> game_state;
class LobbyClient;
extern std::shared_ptr<LobbyClient> lobby_client;
enum class Resource
{
    Any = 0, Air = 1, Water = 2, Earth = 3, Fire = 4, Aether = 5
};
constexpr int NUM_RESOURCES = 6;
extern SDL_Texture *t_resource[NUM_RESOURCES];
extern SDL_Texture *t_game_view, *t_options_view, *t_text_view, *t_time_remaining, *t_actions_left, *t_cards_in_deck;
extern SDL_Texture *t_play_card, *t_discard_card, *t_action_attack, *t_action_move;
extern SDL_Texture *t_unit_health, *t_unit_attack, *t_unit_defense, *t_unit_range, *t_unit_speed;
extern SDL_Texture *t_end_turn, *t_draw_card;
void init_misc_sprites();
class GameState;
class Selectable
{
public:
    std::vector<std::function<void()> > deselect_funcs;
    virtual void operate_selected() = 0;
    virtual void render_selected(const GameState &game_state, int x, int y, int fsz) = 0;
};
