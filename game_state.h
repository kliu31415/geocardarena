#pragma once
#include <vector>
#include <memory>
#include <utility>
#include "unit.h"
#include "player.h"
#include "fundamentals.h"
#include "static_array.h"
#include "temporary_mod.h"
class GameState
{
    double game_start_time;
    int turn_number;
    int current_player_turn; //which player's turn is it right now? How many actions do they have left this turn?
    double turn_length, turn_start_time;
    //if players is ever resized then a bunch of iterators are invalidated which causes issues, which is why vector is not used here
    static_array<Player> players;
    std::vector<SDL_Event> input_events;
    void reset();
    void getSelectedTile(Player &player, int &tileX, int &tileY) const;
    bool is_in_map(int tileX, int tileY) const;
    std::shared_ptr<Unit> unit_at_tile(int tileX, int tileY) const;
    void select_unit(Player &p, const std::shared_ptr<Unit> &unit);
    void select_card(Player &p, int cardnum);
    void handle_mouse_click_game();
    void handle_mouse_click();
    void units_fight(Unit *a, Unit *b);
public:
    constexpr static int BOARD_W = 20, BOARD_H = 20;
    int num_players, player_actions_left;
    mutable std::vector<InfoBox> info_boxes;
    std::vector<std::shared_ptr<Unit> > units;
    std::vector<TemporaryMod> tmods;
    GameState(int num_players);
    void operate();
    void render();
    void initNewGame(int num_players);
    void goToNextTurn();
    void drawCard();
    static SDL_Rect getCardRect(int card_num);
    bool unit_receive_damage(Unit *x, int dmg); //returns true if the unit dies
};
