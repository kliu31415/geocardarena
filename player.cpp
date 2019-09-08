#include <algorithm>
#include <queue>
#include <utility>
#include <memory>
#include "player.h"
#include "fundamentals.h"
#include "unit.h"
#include "spell.h"
#include "game_state.h"
#include "menu.h"
using namespace std;
using SA2d_bool = static_array<static_array<bool> >;

Player::Player()
{
    menuStack = this->menu_stack = make_shared<std::stack<std::shared_ptr<Menu> > >();
    this->menu_stack->push(in_game_menu::main);
    this->action = Action::None;
    this->selected = nullptr;
    this->show_info_boxes = false;
    this->cameraX = this->cameraY = 0;
    this->zoom = 1;
    this->game_view = GameView::game;
    for(auto &i: this->resources)
        i = 0;
    this->can_do_at_tile = SA2d_bool(GameState::BOARD_W);
    for(auto &i: can_do_at_tile)
        i = static_array<bool>(GameState::BOARD_H);
}
SDL_Color Player::getColor() const
{
    switch(ID)
    {
    case 1:
        return SDL_Color{255, 0, 0};
        break;
    case 2:
        return SDL_Color{0, 255, 0};
        break;
    case 3:
        return SDL_Color{0, 0, 255};
        break;
    case 4:
        return SDL_Color{170, 85, 0};
        break;
    default:
        print_warning("unrecognized player ID: " + to_str(ID));
        return SDL_Color{0, 0, 0};
    }
}
void Player::update_input_vars()
{
    window_w = getWindowW();
    window_h = getWindowH();
    mouse_x = getMouseX();
    mouse_y = getMouseY();
    text_size_mult = sdl_settings::textSizeMult;
}
int Player::get_font_size(double sz) const
{
    return text_size_mult * std::pow(2.0, sz) * std::sqrt(window_w * window_h) / 50;
}
bool Player::tryToDrawCard()
{
    if(!cards_in_deck.empty() && getNumCardsInHand() < MAX_CARDS_IN_HAND)
    {
        for(auto &i: cards_in_hand)
        {
            if(i == nullptr)
            {
                i = move(cards_in_deck.front());
                break;
            }
        }
        cards_in_deck.pop_front();
        return true;
    }
    return false;
}
int Player::getNumCardsInHand() const
{
    int cnt = 0;
    for(auto &i: cards_in_hand)
        if(i != nullptr)
            cnt++;
    return cnt;
}
void Player::select(const shared_ptr<Selectable> &to_select)
{
    if(selected != nullptr)
        for(auto &i: selected->deselect_funcs)
            i();
    selected = to_select;
    if(selected != nullptr)
        selected->deselect_funcs.clear();
}
void Player::create_clickable(GameView view, const std::shared_ptr<Clickable> clickable)
{
    clickables[(int)view].emplace_back(clickable);
}
void Player::clean_clickables()
{
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<(int)clickables[i].size(); j++)
        {
            if(clickables[i][j]->should_destroy())
            {
                clickables[i].erase(clickables[i].begin() + j);
                j--;
                continue;
            }
        }
    }
}
bool Player::can_afford(Card *card)
{
    for(int i=0; i<NUM_RESOURCES; i++)
        if(resources[i] < Card::COST[card->type][i])
            return false;
    return true;
}
void Player::sub_res(Card *card)
{
    for(int i=0; i<NUM_RESOURCES; i++)
        resources[i] -= Card::COST[card->type][i];
}
void Player::add_res(Resource res)
{
    resources[(int)res]++;
}
void Player::exchange_for_any(Resource res)
{
    if(resources[(int)res] > 0)
    {
        resources[(int)res]--;
        resources[(int)Resource::Any]++;
    }
}
static void bfs_set(SA2d_bool &occ, SA2d_bool &to_set, queue<pii> &cur, int d)
{
    for(int i=0; i<=d && !cur.empty(); i++)
    {
        queue<pii> nxt;
        while(!cur.empty())
        {
            int x = cur.front().first, y = cur.front().second;
            cur.pop();
            if(x<0 || y<0 || x>=GameState::BOARD_W || y>=GameState::BOARD_H || (i!=0 && occ[x][y]) || to_set[x][y])
                continue;
            if(i != 0)
            {
                to_set[x][y] = true;
            }
            nxt.emplace(x-1, y);
            nxt.emplace(x+1, y);
            nxt.emplace(x, y-1);
            nxt.emplace(x, y+1);
        }
        cur = move(nxt);
    }
}
static SA2d_bool get_occupied_locations(GameState &game_state)
{
    auto occ = SA2d_bool(GameState::BOARD_W);
    for(auto &i: occ)
    {
        i = static_array<bool>(GameState::BOARD_H);
        for(auto &j: i)
            j = false;
    }
    for(auto &i: game_state.units)
        occ[i->x][i->y] = true;
    return occ;
}
void Player::compute_where_unit_can_move(Unit &unit)
{
    for(auto &i: can_do_at_tile)
        for(auto &j: i)
            j = false;
    auto occ = get_occupied_locations(*game_state);
    queue<pii> src;
    src.emplace(unit.x, unit.y);
    bfs_set(occ, can_do_at_tile, src, unit.movement_left);
}
void Player::compute_where_unit_can_place(Unit &unit)
{
    for(auto &i: can_do_at_tile)
        for(auto &j: i)
            j = false;
    auto occ = get_occupied_locations(*game_state);
    queue<pii> src;
    for(auto &i: game_state->units)
    {
        if(i->player_owner == unit.player_owner && (!unit.hasAttribute(Unit::Attribute::Unit) || i->hasAttribute(Unit::Attribute::Deployment)))
        {
            src.emplace(i->x, i->y);
        }
    }
    bfs_set(occ, can_do_at_tile, src, Player::DEPLOYMENT_RADIUS);
}
void Player::compute_where_unit_can_attack(Unit &unit)
{
    for(auto &i: can_do_at_tile)
        for(auto &j: i)
            j = false;
    for(auto &i: game_state->units)
        if(i->player_owner != unit.player_owner && unit.dist(i.get()) <= unit.range)
            can_do_at_tile[i->x][i->y] = true;
}
void Player::compute_where_spell_can_cast(int spelltype)
{
    for(auto &i: can_do_at_tile)
        for(auto &j: i)
            j = false;
    switch(Spell::INPUT_DATA_TYPE[spelltype])
    {
    case Spell::Data::Type::unit:
        for(auto &i: game_state->units)
            can_do_at_tile[i->x][i->y] = true;
        break;
    default:
        print_error("spell input data type " + to_str((int)Spell::INPUT_DATA_TYPE[spelltype]) + " not supported");
        break;
    }
}
void Player::render_can_do_at_tile()
{
    double TILE_SIZE = window_h / (double)GameState::BOARD_H;
    for(int i=0; i<GameState::BOARD_W; i++)
    {
        for(int j=0; j<GameState::BOARD_H; j++)
        {
            if(can_do_at_tile[i][j])
                fillRect(i * TILE_SIZE, j * TILE_SIZE, TILE_SIZE, TILE_SIZE, 255, 255, 0, 30);
        }
    }
}
