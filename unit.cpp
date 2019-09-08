#include "unit.h"
#include "player.h"
#include "game_state.h"
#include <string>
#include <vector>
#include <algorithm>
using namespace std;
int Unit::IDcounter = 0;
void (*Unit::BEGIN_ROT_FUNC[NUM_UNITS])(GameState &game_state, const shared_ptr<Unit> &unit);
int Unit::HEALTH[NUM_UNITS], Unit::SPEED[NUM_UNITS], Unit::ATTACK[NUM_UNITS], Unit::DEFENSE[NUM_UNITS], Unit::RANGE[NUM_UNITS];
vector<Unit::Attribute> Unit::ATTRIBUTES[NUM_UNITS];
std::vector<std::pair<Unit::Attribute, int> > Unit::BONUS_ATTACK[NUM_UNITS];

Unit::Unit(int type, Player *player_owner, int x, int y): Card(player_owner, type)
{
    this->ID = IDcounter++;
    this->x = x;
    this->y = y;
    this->health = HEALTH[type];
    this->movement_left = SPEED[type];
    this->attack = ATTACK[type];
    this->range = RANGE[type];
    this->speed = SPEED[type];
    this->defense = DEFENSE[type];
    this->already_paid_to_move = this->already_attacked = false;
}
void Unit::init(bool load_sprites)
{
    constexpr int Any = (int)Resource::Any, Air = (int)Resource::Air, Water = (int)Resource::Water;
    constexpr int Earth = (int)Resource::Earth, Fire = (int)Resource::Fire, Aether = (int)Resource::Aether;
    using A = Attribute;
    int cur;
    //
    cur = 2;
    NAME[cur] = "Base";
    HEALTH[cur] = 40;
    ATTACK[cur] = 0;
    DEFENSE[cur] = 0;
    SPEED[cur] = 0;
    RANGE[cur] = 0;
    setAttributes(cur, A::Building, A::Deployment);
    DESCRIPTION[cur] = "A player's center of operations. A player loses when their base is destroyed.";
    //
    cur = 3;
    NAME[cur] = "Hill";
    HEALTH[cur] = 5;
    ATTACK[cur] = 0;
    DEFENSE[cur] = 0;
    SPEED[cur] = 0;
    RANGE[cur] = 0;
    setAttributes(cur, A::Building, A::Economic);
    DESCRIPTION[cur] = "Grants +1 Air at turn start.";
    BEGIN_ROT_FUNC[cur] = [](GameState&, const shared_ptr<Unit> &unit) -> void {
        unit->player_owner->add_res(Resource::Air);
    };
    //
    cur = 4;
    NAME[cur] = "Pond";
    HEALTH[cur] = 5;
    ATTACK[cur] = 0;
    DEFENSE[cur] = 0;
    SPEED[cur] = 0;
    RANGE[cur] = 0;
    setAttributes(cur, A::Building, A::Economic);
    DESCRIPTION[cur] = "Grants +1 Water at turn start.";
    BEGIN_ROT_FUNC[cur] = [](GameState&, const shared_ptr<Unit> &unit) -> void {
        unit->player_owner->add_res(Resource::Water);
    };
    //
    cur = 5;
    NAME[cur] = "Volcano";
    HEALTH[cur] = 5;
    ATTACK[cur] = 0;
    DEFENSE[cur] = 0;
    SPEED[cur] = 0;
    RANGE[cur] = 0;
    setAttributes(cur, A::Building, A::Economic);
    DESCRIPTION[cur] = "Grants +1 Fire at turn start.";
    BEGIN_ROT_FUNC[cur] = [](GameState&, const shared_ptr<Unit> &unit) -> void {
        unit->player_owner->add_res(Resource::Fire);
    };
    //
    cur = 6;
    NAME[cur] = "Meadow";
    HEALTH[cur] = 5;
    ATTACK[cur] = 0;
    DEFENSE[cur] = 0;
    SPEED[cur] = 0;
    RANGE[cur] = 0;
    setAttributes(cur, A::Building, A::Economic);
    DESCRIPTION[cur] = "Grants +1 Earth at turn start.";
    BEGIN_ROT_FUNC[cur] = [](GameState&, const shared_ptr<Unit> &unit) -> void {
        unit->player_owner->add_res(Resource::Earth);
    };
    //
    cur = 7;
    NAME[cur] = "Shrine";
    HEALTH[cur] = 4;
    ATTACK[cur] = 0;
    DEFENSE[cur] = 0;
    SPEED[cur] = 0;
    RANGE[cur] = 0;
    setAttributes(cur, A::Building, A::Economic);
    DESCRIPTION[cur] = "Grants +1 Aether at turn start.";
    BEGIN_ROT_FUNC[cur] = [](GameState&, const shared_ptr<Unit> &unit) -> void {
        unit->player_owner->add_res(Resource::Aether);
    };
    //
    cur = 15;
    NAME[cur] = "Badlands";
    HEALTH[cur] = 6;
    ATTACK[cur] = 0;
    DEFENSE[cur] = 0;
    SPEED[cur] = 0;
    RANGE[cur] = 0;
    setAttributes(cur, A::Building, A::Economic);
    DESCRIPTION[cur] = "Grants +1 Any at turn start.";
    BEGIN_ROT_FUNC[cur] = [](GameState&, const shared_ptr<Unit> &unit) -> void {
        unit->player_owner->add_res(Resource::Any);
    };
    //
    cur = 24;
    NAME[cur] = "Scout";
    HEALTH[cur] = 1;
    ATTACK[cur] = 1;
    DEFENSE[cur] = 0;
    SPEED[cur] = 6;
    RANGE[cur] = 1;
    setAttributes(cur, A::Unit, A::Melee);
    COST[cur][Any] = 1;
    DESCRIPTION[cur] = "A weak, cheap, and fast unit.";
    //
    cur = 47;
    NAME[cur] = "Battering Ram";
    HEALTH[cur] = 6;
    ATTACK[cur] = 3;
    DEFENSE[cur] = 2;
    SPEED[cur] = 5;
    RANGE[cur] = 1;
    setAttributes(cur, A::Unit, A::Melee, A::Siege);
    COST[cur][Any] = 4;
    BONUS_ATTACK[cur].emplace_back(A::Building, 5);
    DESCRIPTION[cur] = "A moderate-speed siege weapon. +5 bonus attack against buildings.";
    //
    if(load_sprites)
    {
        for(int i=0; i<NUM_UNITS; i++)
        {
            if(NAME[i] != "")
                SPRITE[i] = loadTexture(("data/unit/" + to_str(i) + ".png").c_str(), 255, 0, 128);
        }
    }
}
void Unit::render(int x, int y, int w, int h) const
{
    SDL_Color color = player_owner->getColor();
    fillRect(x, y, w, h, color.r, color.g, color.b, 15);
    renderCopy(SPRITE[type], x, y, w, h);
    fillRect(x, y, w, h, color.r, color.g, color.b, 15);
}
int Unit::dist(int x, int y)
{
    return abs(x - this->x) + abs(y - this->y);
}
int Unit::dist(Unit *other)
{
    return abs(x - other->x) + abs(y - other->y);
}
bool Unit::hasAttribute(Attribute a) const
{
    return find(ATTRIBUTES[type].begin(), ATTRIBUTES[type].end(), a) != ATTRIBUTES[type].end();
}
void Unit::operate_selected()
{

}
void Unit::draw_stat(const GameState &game_state, int stat, SDL_Texture *t_stat, int origX, int &x, int &y, int fsz)
{
    int font_h = getFontH(fsz);
    if(x + fsz * 2 > getWindowW())
    {
        x = origX;
        y += font_h;
    }
    renderCopy(t_stat, x, y, font_h, font_h);
    if(mouseInRect(x, y, font_h, font_h))
    {
        string desc;
        if(t_stat == t_unit_attack)
            desc = "Attack";
        if(t_stat == t_unit_defense)
            desc = "Defense";
        if(t_stat == t_unit_range)
            desc = "Range";
        if(t_stat == t_unit_speed)
            desc = "Movement left";
        game_state.info_boxes.emplace_back(desc, getMouseX(), getMouseY(), INF, fsz, 200, 200, 200, 240);
    }
    x += font_h;
    drawText(to_str(stat), x, y, fsz);
    x += fsz * 3;
}
void Unit::render_selected(const GameState &game_state, int x, int y, int fsz)
{
    int font_h = getFontH(fsz);
    int origX = x;
    SDL_Color p_color = player_owner->getColor();
    drawText("P" + to_str(player_owner->ID), x, y, fsz, p_color.r, p_color.g, p_color.b, 150);
    x += 2 * fsz;
    renderCopy(SPRITE[type], x, y, font_h, font_h);
    x += font_h;
    drawText(NAME[type], x, y, fsz);
    x += fsz * 9;
    if(x > getWindowW())
    {
        x = origX;
        y += font_h;
    }
    renderCopy(t_unit_health, x, y, font_h, font_h);
    if(mouseInRect(x, y, font_h, font_h))
        game_state.info_boxes.emplace_back("Health", getMouseX(), getMouseY(), INF, fsz, 200, 200, 200, 240);
    x += font_h;
    drawText(to_str(health) + "/" + to_str(HEALTH[type]), x, y, fsz);
    x = origX;
    y += font_h * 2;
    draw_stat(game_state, attack, t_unit_attack, origX, x, y, fsz);
    draw_stat(game_state, defense, t_unit_defense, origX, x, y, fsz);
    draw_stat(game_state, range, t_unit_range, origX, x, y, fsz);
    //draw_stat(game_state, movement_left, t_unit_speed, origX, x, y, fsz);
    renderCopy(t_unit_speed, x, y, font_h, font_h);
    if(mouseInRect(x, y, font_h, font_h))
        game_state.info_boxes.emplace_back("Speed", getMouseX(), getMouseY(), INF, fsz, 200, 200, 200, 240);
    x += font_h;
    drawText(to_str(movement_left) + "/" + to_str(speed), x, y, fsz);
    x = origX;
    y += font_h;
    drawMultilineTextUnbroken(DESCRIPTION[type], x, y, getWindowW() - x, fsz);
}
bool Unit::can_attack()
{
    return !already_attacked && RANGE[type] > 0;
}
bool Unit::can_move()
{
    return movement_left > 0;
}
void Unit::begin_owner_turn(GameState &game_state)
{
    already_paid_to_move = already_attacked = false;
    if(BEGIN_ROT_FUNC[type] != nullptr)
        BEGIN_ROT_FUNC[type](game_state, shared_from_this());
    for(auto i = tmods.begin(); i != tmods.end(); )
    {
        i->turns_left--;
        if(i->turns_left <= 0)
        {
            i = tmods.erase(i);
        }
        else i++;
    }
    recalc_stats();
    movement_left = max(speed, 0);
}
void Unit::recalc_stats()
{
    using Tmod = TemporaryMod;
    int origSpeed = speed;
    auto gs_tmods = player_owner->game_state->tmods;
    speed = TemporaryMod::get_val(Unit::SPEED[type], Tmod::Affects::UnitSpeed, tmods, gs_tmods);
    range = TemporaryMod::get_val(Unit::RANGE[type], Tmod::Affects::UnitRange, tmods, gs_tmods);
    attack = TemporaryMod::get_val(Unit::ATTACK[type], Tmod::Affects::UnitAttack, tmods, gs_tmods);
    defense = TemporaryMod::get_val(Unit::DEFENSE[type], Tmod::Affects::UnitDefense, tmods, gs_tmods);
    movement_left = max(0, movement_left + speed - origSpeed);
}
