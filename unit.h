#pragma once
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include "sdl_base.h"
#include "sdl_graphics.h"
#include "card.h"
#include "temporary_mod.h"

class Player;
class GameState;

class Unit: public Card, public std::enable_shared_from_this<Unit>
{
public:
    friend class Card;
    constexpr static int NUM_UNITS = 1000;
    enum class Attribute
    {
        Building, Deployment, Economic, Unit, Wall, Vigilant, Melee, Ranged, Siege
    };
    static int IDcounter;
    static void (*BEGIN_ROT_FUNC[NUM_UNITS])(GameState &game_state, const std::shared_ptr<Unit> &unit);
    int ID;
    int health, speed, attack, defense, range;
    int affiliation, x, y;
    std::vector<TemporaryMod> tmods;
    bool already_paid_to_move, already_attacked;
    int movement_left;
    static void init(bool load_sprites);
    Unit(int type, Player *player_owner, int x, int y);
    void render(int x, int y, int w, int h) const override;
    int dist(Unit *other);
    int dist(int x, int y);
    bool hasAttribute(Attribute a) const;
    virtual void operate_selected() override;
    virtual void render_selected(const GameState &game_state, int x, int y, int fsz) override;
    bool can_attack();
    bool can_move();
    void begin_owner_turn(GameState &game_state);
    void recalc_stats();
    template<class ...Args> void add_tmod(const Args &...args)
    {
        tmods.emplace_back(args...);
        recalc_stats();
    }
    static std::vector<std::pair<Attribute, int> > BONUS_ATTACK[NUM_UNITS];
private:
    static int HEALTH[NUM_UNITS], SPEED[NUM_UNITS], ATTACK[NUM_UNITS], DEFENSE[NUM_UNITS], RANGE[NUM_UNITS];
    static std::vector<Attribute> ATTRIBUTES[NUM_UNITS];
    static void draw_stat(const GameState &game_state, int stat, SDL_Texture *t_stat, int origX, int &x, int &y, int fsz);
    static void setAttributes(int pos, Attribute x)
    {
        ATTRIBUTES[pos].push_back(x);
    }
    template<class... Args> static void setAttributes(int pos, Attribute x, Args... args)
    {
        ATTRIBUTES[pos].push_back(x);
        setAttributes(pos, args...);
    }
};
