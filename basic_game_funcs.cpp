#include "sdl_base.h"
#include "basic_game_funcs.h"
bool SHOW_EXTRA_INFO = true;
std::shared_ptr<std::stack<std::shared_ptr<Menu> > > global_menu_stack;
std::shared_ptr<GameState> game_state;
std::shared_ptr<LobbyClient> lobby_client;
global_game_state current_global_game_state = global_game_state::menu;
SDL_Texture *t_resource[NUM_RESOURCES];
SDL_Texture *t_game_view, *t_options_view, *t_text_view, *t_time_remaining, *t_actions_left, *t_cards_in_deck;
SDL_Texture *t_play_card, *t_discard_card, *t_action_attack, *t_action_move;
SDL_Texture *t_unit_health, *t_unit_attack, *t_unit_defense, *t_unit_range, *t_unit_speed;
SDL_Texture *t_end_turn, *t_draw_card;
void init_resource_sprites()
{
    t_resource[(int)Resource::Any] = loadTexture("data/resource/any.png", 0, 0, 0);
    t_resource[(int)Resource::Air] = loadTexture("data/resource/air.png", 0, 0, 0);
    t_resource[(int)Resource::Water] = loadTexture("data/resource/water.png", 255, 255, 255);
    t_resource[(int)Resource::Fire] = loadTexture("data/resource/fire.png", 255, 255, 255);
    t_resource[(int)Resource::Earth] = loadTexture("data/resource/earth.png", 255, 255, 255);
    t_resource[(int)Resource::Aether] = loadTexture("data/resource/aether.png", 0, 0, 0);
}
void init_misc_sprites()
{
    //unit stats
    t_unit_health = loadTexture("data/misc_sprites/unit_health.png", 255, 255, 255);
    t_unit_attack = loadTexture("data/misc_sprites/unit_attack.png", 255, 255, 255);
    t_unit_defense = loadTexture("data/misc_sprites/unit_defense.png", 255, 255, 255);
    t_unit_speed = loadTexture("data/misc_sprites/unit_speed.png", 255, 255, 255);
    t_unit_range = loadTexture("data/misc_sprites/unit_range.png", 255, 255, 255);
    //row 1
    t_game_view = loadTexture("data/misc_sprites/game_view.png", 255, 255, 255);
    t_text_view = loadTexture("data/misc_sprites/text_view.png", 255, 255, 255);
    t_options_view = loadTexture("data/misc_sprites/options_view.png", 255, 255, 255);
    t_time_remaining = loadTexture("data/misc_sprites/time_remaining.png", 255, 255, 255);
    t_actions_left = loadTexture("data/misc_sprites/actions_left.png", 255, 255, 255);
    //row 2
    init_resource_sprites();
    //row 3
    t_end_turn = loadTexture("data/misc_sprites/end_turn.png", 255, 255, 255);
    t_draw_card = loadTexture("data/misc_sprites/draw_card.png", 255, 255, 255);
    t_discard_card = loadTexture("data/misc_sprites/discard_card.png", 255, 255, 255);
    t_cards_in_deck = loadTexture("data/misc_sprites/cards_in_deck.png", 255, 255, 255);
    //
    t_play_card = loadTexture("data/misc_sprites/play_card.png", 255, 255, 255);
    t_action_attack = loadTexture("data/misc_sprites/action_attack.png", 255, 255, 255);
    t_action_move = loadTexture("data/misc_sprites/action_move.png", 255, 255, 255);
}
