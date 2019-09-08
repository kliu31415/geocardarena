#pragma once
#include "basic_game_funcs.h"
#include "fundamentals.h"
#include "sdl_graphics.h"
#include "card.h"
#include "static_array.h"
#include <memory>
#include <vector>
#include <deque>
#include <cstdint>

class Spell;
class Menu;
class Unit;
class GameState;
struct Player
{
    enum class Action{None, UnitPlace, UnitMove, UnitAttack, SpellCast} action;
    enum class GameView{global = 0, game = 1, text = 2, options = 3} game_view;
    constexpr static int DEPLOYMENT_RADIUS = 2;
    constexpr static int MAX_CARDS_IN_HAND = 10;
    double text_size_mult;
    int16_t window_w, window_h, mouse_x, mouse_y;
    std::shared_ptr<std::stack<std::shared_ptr<Menu> > > menu_stack;
    bool show_info_boxes;
    double cameraX, cameraY, zoom;
    int resources[NUM_RESOURCES];
    std::vector<std::shared_ptr<Clickable> > clickables[4];
    std::shared_ptr<Card> cards_in_hand[MAX_CARDS_IN_HAND];
    std::deque<std::shared_ptr<Card> > cards_in_deck;
    std::shared_ptr<Selectable> selected;
    GameState *game_state;
    //"do" is anything binary displayed on the board tiles, e.g. can you place a unit here, can a unit move here
    //we can use just 1 2d array to save memory because we'll only ever need to show 1 "do" at a time
    static_array<static_array<bool> > can_do_at_tile;
    int ID;
    Player();
    SDL_Color getColor() const;
    void update_input_vars();
    int get_font_size(double sz) const;
    bool tryToDrawCard(); //returns true if a card was successfully drawn (fails if deck is empty, player's hand is full, ...)
    int getNumCardsInHand() const;
    void select(const std::shared_ptr<Selectable> &to_select);
    void create_clickable(GameView view, const std::shared_ptr<Clickable> clickable);
    void clean_clickables();
    bool can_afford(Card *card);
    void sub_res(Card *card);
    void add_res(Resource res);
    void exchange_for_any(Resource res);
    void compute_where_unit_can_move(Unit &unit);
    void compute_where_unit_can_place(Unit &unit);
    void compute_where_unit_can_attack(Unit &unit);
    void compute_where_spell_can_cast(int spelltype);
    void render_can_do_at_tile();
};
