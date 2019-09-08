#pragma once
#include "basic_game_funcs.h"
#include "fundamentals.h"
#include <string>
#include <memory>
class GameState;
class Player;
class Card: public Selectable
{
public:
    constexpr static int NUM_CARDS = 2000;
    int type;
    Player *player_owner;
    Card(Player *player_owner, int type);
    virtual void operate_selected() override;
    virtual void render_selected(const GameState &game_state, int x, int y, int fsz) override;
    virtual void render(int x, int y, int w, int h) const;
    static SDL_Texture *SPRITE[NUM_CARDS];
    static int COST[NUM_CARDS][NUM_RESOURCES];
    static int ACTION_COST[NUM_CARDS];
    static void init();
protected:
    static std::string NAME[NUM_CARDS], DESCRIPTION[NUM_CARDS];
};
