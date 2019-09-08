#include "game_state.h"
#include "sdl_base.h"
#include "basic_game_funcs.h"
#include "spell.h"
#include "menu.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>
using namespace std;
GameState::GameState(int num_players)
{
    print("starting game with " + to_str(num_players) + " players");
    GameState::initNewGame(num_players);
}
void GameState::reset()
{
    game_start_time = getTicksS();
    units.clear();
    current_player_turn = 0;
    players.clear();
    turn_number = 1;
}
void GameState::getSelectedTile(Player &player, int &tileX, int &tileY) const
{
    double TILE_SIZE = player.window_h / (double)BOARD_H;
    tileX = player.mouse_x / TILE_SIZE;
    tileY = player.mouse_y / TILE_SIZE;
}
bool GameState::is_in_map(int tileX, int tileY) const
{
    return tileX>=0 && tileY>=0 && tileX<BOARD_W && tileY<BOARD_H;
}
shared_ptr<Unit> GameState::unit_at_tile(int tileX, int tileY) const
{
    for(auto &i: units)
        if(i->x == tileX && i->y == tileY)
            return i;
    return nullptr;
}
void GameState::select_unit(Player &p, const std::shared_ptr<Unit> &unit)
{
    p.select(unit);
    if(unit->can_attack()) //attack button
    {
        function<void()> on_click = [=, &p = players[current_player_turn]]() -> void {
            if(p.action != Player::Action::UnitAttack)
            {
                p.action = Player::Action::UnitAttack;
                p.compute_where_unit_can_attack(*unit);
            }
            else
            {
                p.action = Player::Action::None;
            }
        };
        //
        function<int()> x_func = [&p = players[current_player_turn]]() -> int {return p.window_h + getFontH(p.get_font_size(0));};
        function<int()> y_func = [&p = players[current_player_turn]]() -> int {return getFontH(p.get_font_size(0)) * 4;};
        function<int()> s_func = [&p = players[current_player_turn]]() -> int {return getFontH(p.get_font_size(0));};
        auto button = make_shared<Button>(on_click, nullptr, "Attack", t_action_attack, x_func, y_func, s_func, s_func);
        button->border_size = 0.05;
        button->should_shade = [&p = players[current_player_turn]]() -> bool {return p.action == Player::Action::UnitAttack;};
        //possible dangling pointer if button has already been deleted
        unit->deselect_funcs.push_back([=]() -> void {button->should_destroy_ = true;});
        p.create_clickable(Player::GameView::game, move(button));
    }
    if(unit->can_move()) //move button
    {
        function<void()> on_click = [=, &p = players[current_player_turn]]() -> void {
            if(p.action != Player::Action::UnitMove)
            {
                p.action = Player::Action::UnitMove;
                p.compute_where_unit_can_move(*unit);
            }
            else p.action = Player::Action::None;
        };
        //
        function<int()> x_func = [&p = players[current_player_turn]]() -> int {return p.window_h;};
        function<int()> y_func = [&p = players[current_player_turn]]() -> int {return getFontH(p.get_font_size(0)) * 4;};
        function<int()> s_func = [&p = players[current_player_turn]]() -> int {return getFontH(p.get_font_size(0));};
        auto button = make_shared<Button>(on_click, nullptr, "Move", t_action_move, x_func, y_func, s_func, s_func);
        button->border_size = 0.05;
        button->should_shade = [&p = players[current_player_turn]]() -> bool {return p.action == Player::Action::UnitMove;};
        //possible dangling pointer if button has already been deleted
        unit->deselect_funcs.push_back([=]() -> void {button->should_destroy_ = true;});
        p.create_clickable(Player::GameView::game, move(button));
    }
}
void GameState::select_card(Player &p, int cardnum)
{
    //play card button
    p.select(p.cards_in_hand[cardnum]);
    function<void()> on_click;
    int type = p.cards_in_hand[cardnum]->type;
    if(type < Unit::NUM_UNITS) //it's a unit
    {
        on_click = [=, &p = players[current_player_turn]]() -> void {
            if(p.action != Player::Action::UnitPlace)
            {
                if(p.can_afford(p.cards_in_hand[cardnum].get()) && player_actions_left > Card::ACTION_COST[type])
                {
                    p.action = Player::Action::UnitPlace;
                    Unit unit(type, p.cards_in_hand[cardnum]->player_owner, NOT_SET, NOT_SET);
                    p.compute_where_unit_can_place(unit);
                }
            }
            else p.action = Player::Action::None;
        };
    }
    else //it's a spell
    {
        switch(Spell::INPUT_DATA_TYPE[type])
        {
        case Spell::Data::Type::none:
            on_click = [=, &p = players[current_player_turn]]() -> void {
                if(p.can_afford(p.cards_in_hand[cardnum].get()) && player_actions_left > Card::ACTION_COST[type])
                {
                    Spell::Data data_none;
                    Spell::Data d(Spell::Data::Type::none, data_none);
                    auto s = make_shared<Spell>(&p, type, d); //need a shared pointer to ensure shared_from_this is valid
                    s->cast();
                    p.cards_in_hand[cardnum] = nullptr;
                    p.select(nullptr);
                    player_actions_left -= Card::ACTION_COST[type];
                }
            };
            break;
        case Spell::Data::Type::unit:
        case Spell::Data::Type::square:
            on_click = [=, &p = players[current_player_turn]]() -> void {
                if(p.action != Player::Action::SpellCast)
                {
                    if(p.can_afford(p.cards_in_hand[cardnum].get()) && player_actions_left > Card::ACTION_COST[type])
                    {
                        p.action = Player::Action::SpellCast;
                        p.compute_where_spell_can_cast(type);
                    }
                }
                else p.action = Player::Action::None;
            };
            break;
        }
    }
    function<int()> x_func = [&p = players[current_player_turn]]() -> int {return p.window_h;};
    function<int()> y_func = [&p = players[current_player_turn]]() -> int {return getFontH(p.get_font_size(0)) * 4;};
    function<int()> s_func = [&p = players[current_player_turn]]() -> int {return getFontH(p.get_font_size(0));};
    auto button = make_shared<Button>(on_click, nullptr, "Play card", t_play_card, x_func, y_func, s_func, s_func);
    button->border_size = 0.05;
    if(type < Unit::NUM_UNITS)
        button->should_shade = [&p = players[current_player_turn]]() -> bool {return p.action == Player::Action::UnitPlace;};
    else
        button->should_shade = [&p = players[current_player_turn]]() -> bool {return p.action == Player::Action::SpellCast;};
    p.cards_in_hand[cardnum]->deselect_funcs.push_back([=]() -> void {button->should_destroy_ = true;});
    p.create_clickable(Player::GameView::game, move(button));
    //discard card button
    on_click = [=, &p = players[current_player_turn]]() -> void {
        p.cards_in_hand[cardnum] = nullptr;
        p.select(nullptr);
    };
    x_func = [&p = players[current_player_turn]]() -> int {return p.window_h + getFontH(p.get_font_size(0));};
    button = make_shared<Button>(on_click, nullptr, "Discard card", t_discard_card, x_func, y_func, s_func, s_func);
    button->border_size = 0.05;
    p.cards_in_hand[cardnum]->deselect_funcs.push_back([=]() -> void {button->should_destroy_ = true;});
    p.create_clickable(Player::GameView::game, move(button));
}
void GameState::handle_mouse_click_game()
{
    auto &p = players[current_player_turn];
    switch(p.action)
    {
    case Player::Action::None:{
        int INFO_OFFSET = p.window_h;
        if(p.mouse_x < INFO_OFFSET) //the player clicked on a tile on the board
        {
            p.select(nullptr);
            int tileX, tileY;
            GameState::getSelectedTile(p, tileX, tileY);
            for(auto &i: units)
            {
                if(i->x==tileX && i->y==tileY)
                {
                    select_unit(p, i);
                    break;
                }
            }
        }
        for(int i=0; i<Player::MAX_CARDS_IN_HAND; i++) //the player clicked on a card
        {
            if(p.cards_in_hand[i] != nullptr)
            {
                SDL_Rect rect = getCardRect(i);
                if(mouseInRect(&rect))
                {
                    select_card(p, i);
                    break;
                }
            }
        }
        }break;
    case Player::Action::UnitPlace:{
        int INFO_OFFSET = p.window_h;
        if(p.mouse_x < INFO_OFFSET) //the player clicked on a tile on the board
        {
            int tileX, tileY;
            GameState::getSelectedTile(p, tileX, tileY);
            if(p.can_do_at_tile[tileX][tileY])
            {
                Card *card = static_cast<Card*>(p.selected.get());
                p.sub_res(card);
                units.push_back(make_shared<Unit>(card->type, &p, tileX, tileY));
                p.action = Player::Action::None;
                for(auto &i: p.cards_in_hand)
                    if(i == p.selected)
                        i = nullptr;
                p.select(nullptr);
                player_actions_left -= Card::ACTION_COST[card->type];
            }
        }
        }break;
    case Player::Action::UnitMove:{
        int INFO_OFFSET = p.window_h;
        if(p.mouse_x < INFO_OFFSET) //the player clicked on a tile on the board
        {
            Unit *unit = static_cast<Unit*>(p.selected.get());
            if(unit->movement_left > 0 && (unit->already_paid_to_move || player_actions_left>0))
            {
                int tileX, tileY;
                GameState::getSelectedTile(p, tileX, tileY);
                if(p.can_do_at_tile[tileX][tileY])
                {
                    if(!unit->already_paid_to_move)
                    {
                        unit->already_paid_to_move = true;
                        player_actions_left--;
                    }
                    unit->movement_left -= unit->dist(tileX, tileY);
                    unit->x = tileX;
                    unit->y = tileY;
                    if(unit->can_move()) //still has movement left
                    {
                        p.compute_where_unit_can_move(*unit);
                    }
                    else //can't move anymore, so update
                    {
                        p.action = Player::Action::None;
                        //reselect the current unit
                        select_unit(p, static_pointer_cast<Unit>(p.selected));
                    }
                }
            }
        }
        }break;
    case Player::Action::UnitAttack:{
        int INFO_OFFSET = p.window_h;
        if(p.mouse_x < INFO_OFFSET) //the player clicked on a tile on the board
        {
            Unit *unit = static_cast<Unit*>(p.selected.get());
            if(!unit->already_attacked && player_actions_left>0)
            {
                int tileX, tileY;
                GameState::getSelectedTile(p, tileX, tileY);
                if(p.can_do_at_tile[tileX][tileY])
                {
                    auto attacked = unit_at_tile(tileX, tileY);
                    if(attacked == nullptr)
                        print_error("can_do_at_tile[tileX][tileY]==true, but there is no unit there");
                    else
                    {
                        player_actions_left--;
                        unit->already_attacked = true;
                        units_fight(unit, attacked.get());
                        p.action = Player::Action::None;
                        //reselect the current unit to remove the attack button
                        select_unit(p, static_pointer_cast<Unit>(p.selected));
                    }
                }
            }
        }
        }break;
    case Player::Action::SpellCast:{
        int INFO_OFFSET = p.window_h;
        if(p.mouse_x < INFO_OFFSET)
        {
            if(player_actions_left > 0)
            {
                Card *card = static_cast<Card*>(p.selected.get());
                switch(Spell::INPUT_DATA_TYPE[card->type])
                {
                case Spell::Data::Type::unit:
                    int tileX, tileY;
                    getSelectedTile(p, tileX, tileY);
                    if(p.can_do_at_tile[tileX][tileY])
                    {
                        Spell::Data d;
                        unit_at_tile(tileX, tileY);
                        d.unit.unit = unit_at_tile(tileX, tileY);
                        if(d.unit.unit == nullptr)
                            print_error("can_do_at_tile[tileX][tileY]==true, but there is no unit there");
                        else if(d.unit.unit->hasAttribute(Unit::Attribute::Unit))
                        {
                            auto s = make_shared<Spell>(&p, card->type, d); //need a shared_ptr because we use shared_from_this
                            s->cast();
                            for(auto &i: p.cards_in_hand)
                                if(i == p.selected)
                                    i = nullptr;
                            p.select(nullptr);
                            player_actions_left -= Card::ACTION_COST[card->type];
                            p.action = Player::Action::None;
                        }
                    }
                    break;
                default:
                    print_error("spell data type " + to_str((int)Spell::INPUT_DATA_TYPE[card->type]) + " not supported");
                    break;
                }
            }
        }
        }break;
    }
}
void GameState::handle_mouse_click()
{
    auto &p = players[current_player_turn];
    bool already_clicked = false;
    for(auto &i: p.clickables[0])
    {
        if(i->handle_click())
        {
            already_clicked = true;
            break;
        }
    }
    for(auto &i: p.clickables[(int)p.game_view])
    {
        if(i->handle_click())
        {
            already_clicked = true;
            break;
        }
    }
    if(already_clicked)
        return;
    switch(p.game_view)
    {
    case Player::GameView::game:
        handle_mouse_click_game();
        break;
    default:
        break;
    }
}
bool GameState::unit_receive_damage(Unit *x, int dmg)
{
    x->health -= dmg;
    if(x->health <= 0)
    {
        for(auto i=units.begin(); i!=units.end(); i++)
        {
            if(i->get() == x)
            {
                units.erase(i);
                break;
            }
        }
        return true;
    }
    return false;
}
void GameState::units_fight(Unit *a, Unit *b)
{
    int bonus = 0;
    for(auto &i: Unit::BONUS_ATTACK[a->type])
        if(b->hasAttribute(i.first))
            bonus += i.second;
    if(unit_receive_damage(b, max(0, a->attack + bonus - b->defense)))
    {
        bonus = 0;
        for(auto &i: Unit::BONUS_ATTACK[b->type])
            if(a->hasAttribute(i.first))
                bonus += i.second;
        unit_receive_damage(a, max(0, b->attack + bonus - a->defense));
    }
    //TODO: when units are deleted, make sure selected is cleared if it points to them
}
void GameState::render()
{
    auto &p = players[current_player_turn];
    p.update_input_vars();
    SDL_ShowCursor(true);
    double TILE_SIZE = p.window_h / (double)BOARD_H;
    double LINE_WIDTH = TILE_SIZE / 20;
    renderClear(240, 240, 240);
    //units
    for(auto &i: units)
    {
        i->render(i->x * TILE_SIZE, i->y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
        if(p.selected == i)
            fillRect(i->x * TILE_SIZE, i->y * TILE_SIZE, TILE_SIZE, TILE_SIZE, 0, 0, 0, 40 + 40 * std::sin(2*PI * getTicksS()));
        drawText(to_str(i->health), i->x * TILE_SIZE, i->y * TILE_SIZE, TILE_SIZE / 2, 0, 0, 0, 200);
    }
    //stuff based on current action
    switch(p.action)
    {
    case Player::Action::None:
        break;
    case Player::Action::UnitPlace:{
        p.render_can_do_at_tile();
        int tileX, tileY;
        getSelectedTile(p, tileX, tileY);
        if(is_in_map(tileX, tileY))
        {
            if(p.can_do_at_tile[tileX][tileY])
            {
                SDL_Texture *t = Card::SPRITE[static_cast<Card*>(p.selected.get())->type];
                SDL_SetTextureAlphaMod(t, 150);
                renderCopy(t, tileX * TILE_SIZE, tileY * TILE_SIZE, TILE_SIZE, TILE_SIZE);
                SDL_SetTextureAlphaMod(t, 255);
            }
            else fillRect(tileX * TILE_SIZE, tileY * TILE_SIZE, TILE_SIZE, TILE_SIZE, 0, 0, 0, 50);
        }
        }break;
    case Player::Action::UnitMove:{
        Unit *unit = static_cast<Unit*>(p.selected.get());
        if(unit->already_paid_to_move || player_actions_left > 0)
        {
            p.render_can_do_at_tile();
            int tileX, tileY;
            getSelectedTile(p, tileX, tileY);
            if(is_in_map(tileX, tileY))
            {
                if(p.can_do_at_tile[tileX][tileY])
                {
                    renderCopy(t_action_move, tileX * TILE_SIZE, tileY * TILE_SIZE, TILE_SIZE, TILE_SIZE);
                }
                else fillRect(tileX * TILE_SIZE, tileY * TILE_SIZE, TILE_SIZE, TILE_SIZE, 0, 0, 0, 50);
            }
        }
        }break;
    case Player::Action::UnitAttack:{
        Unit *unit = static_cast<Unit*>(p.selected.get());
        if(unit->already_paid_to_move || player_actions_left > 0)
        {
            p.render_can_do_at_tile();
            int tileX, tileY;
            getSelectedTile(p, tileX, tileY);
            if(is_in_map(tileX, tileY))
            {
                if(p.can_do_at_tile[tileX][tileY])
                {
                    renderCopy(t_action_attack, tileX * TILE_SIZE, tileY * TILE_SIZE, TILE_SIZE, TILE_SIZE);
                }
                else fillRect(tileX * TILE_SIZE, tileY * TILE_SIZE, TILE_SIZE, TILE_SIZE, 0, 0, 0, 50);
            }
        }
        }break;
    case Player::Action::SpellCast:{
        Card *card = static_cast<Card*>(p.selected.get());
        if(player_actions_left > 0)
        {
            switch(Spell::INPUT_DATA_TYPE[card->type])
            {
            case Spell::Data::Type::unit:
                p.render_can_do_at_tile();
                int tileX, tileY;
                getSelectedTile(p, tileX, tileY);
                if(is_in_map(tileX, tileY))
                {
                    if(p.can_do_at_tile[tileX][tileY])
                    {
                        renderCopy(Card::SPRITE[card->type], tileX * TILE_SIZE, tileY * TILE_SIZE, TILE_SIZE, TILE_SIZE);
                    }
                    else fillRect(tileX * TILE_SIZE, tileY * TILE_SIZE, TILE_SIZE, TILE_SIZE, 0, 0, 0, 50);
                }
                break;
            default:
                print_error("spell data type " + to_str((int)Spell::INPUT_DATA_TYPE[card->type]) + " not supported");
                break;
            }
        }
        }break;
    }
    //board
    setColor(0, 0, 0);
    vector<SDL_Rect> board_lines;
    for(int i=0; i<=BOARD_W; i++)
    {
        board_lines.push_back({0, (int)(i * TILE_SIZE - LINE_WIDTH / 2), getWindowH(), (int)(LINE_WIDTH)});
    }
    for(int i=0; i<=BOARD_H; i++)
    {
        board_lines.push_back({(int)(i * TILE_SIZE - LINE_WIDTH / 2), 0, (int)(LINE_WIDTH), getWindowH()});
    }
    SDL_RenderFillRects(getRenderer(), &board_lines[0], board_lines.size());
    //first info box rect
    int INFO_OFFSET = getWindowH();
    int INFO_W = getWindowW() - getWindowH();
    int cur_text_y = 0;
    int fsz = getFontSize(0), font_h = getFontH(fsz);
    fillRect(INFO_OFFSET, 0, INFO_W, font_h, 0, 0, 0, 35);
    //resources
    for(int i=0; i<NUM_RESOURCES; i++)
    {
        int xoffset = INFO_OFFSET + i * INFO_W / NUM_RESOURCES;
        auto res_count = to_str(p.resources[i]);
        drawText(res_count, xoffset + font_h, cur_text_y, fsz);
    }
    //2nd info box rect
    cur_text_y += font_h;
    int cur_text_x = INFO_OFFSET + font_h * 3;
    SDL_Color p_color = p.getColor();
    fillRect(cur_text_x, cur_text_y, INFO_W, font_h, p_color.r, p_color.g, p_color.b, 30);
    //player #
    drawText("P" + to_str(p.ID), cur_text_x, cur_text_y, fsz);
    //time left in current player's turn
    cur_text_x += font_h * 2;
    if(mouseInRect(cur_text_x, cur_text_y, font_h, font_h))
        info_boxes.emplace_back("Time left", getMouseX(), getMouseY(), INF, fsz, 200, 200, 200, 240);
    renderCopy(t_time_remaining, cur_text_x, cur_text_y, font_h, font_h);
    double turn_time_left = turn_start_time + turn_length - getTicksS();
    turn_time_left = min(turn_time_left, 599.0); //if it's >= 10 minutes, then just say it's 9:59 to avoid the time from taking too much space
    drawText(seconds_to_str_no_h(ceil(turn_time_left)), cur_text_x + font_h, cur_text_y, fsz);
    //action count
    cur_text_x += fsz * 4;
    if(mouseInRect(cur_text_x, cur_text_y, font_h, font_h))
        info_boxes.emplace_back("Actions left", getMouseX(), getMouseY(), INF, fsz, 200, 200, 200, 240);
    renderCopy(t_actions_left, cur_text_x, cur_text_y, font_h, font_h);
    drawText(to_str(player_actions_left), cur_text_x + font_h, cur_text_y, fsz);
    //cards in deck
    cur_text_x += fsz * 3;
    if(mouseInRect(cur_text_x, cur_text_y, font_h, font_h))
        info_boxes.emplace_back("Cards in deck", getMouseX(), getMouseY(), INF, fsz, 200, 200, 200, 240);
    renderCopy(t_cards_in_deck, cur_text_x, cur_text_y, font_h, font_h);
    drawText(to_str(p.cards_in_deck.size()), cur_text_x + font_h, cur_text_y, fsz);
    //3rd info box rect
    cur_text_y += font_h;
    cur_text_x = INFO_OFFSET;
    fillRect(cur_text_x, cur_text_y, INFO_W, font_h, 0, 0, 0, 35);
    //turn count and game time
    cur_text_x += fsz * 4;
    drawText("T" + to_str(turn_number), cur_text_x, cur_text_y, fsz);
    cur_text_x += fsz * 3;
    drawText(seconds_to_str(getTicksS() - game_start_time), cur_text_x, cur_text_y, fsz);
    //
    cur_text_y += font_h;
    cur_text_x = INFO_OFFSET;
    switch(p.game_view)
    {
    case Player::GameView::game:
        if(p.selected != nullptr) //show info if something is selected
            p.selected->render_selected(*this, cur_text_x, cur_text_y, fsz);
        //cards in hand
        for(size_t i=0; i<Player::MAX_CARDS_IN_HAND; i++)
        {
            SDL_Rect card_rect = getCardRect(i);
            if(p.cards_in_hand[i] != nullptr)
            {
                p.cards_in_hand[i]->render(card_rect.x, card_rect.y, card_rect.w, card_rect.h);
                if(mouseInRect(&card_rect))
                    fillRect(&card_rect, 0, 50, 0, 60);
                if(p.selected == p.cards_in_hand[i])
                    fillRect(&card_rect, 50, 0, 0, 60);
            }
            else
            {
                fillRect(&card_rect, 0, 0, 0, 20);
            }
        }
        break;
    case Player::GameView::text:
        break;
    case Player::GameView::options:{
        ERect menu_rect((int)p.window_h, [&]() -> int {return getFontH(p.get_font_size(0) * 3);},
            p.window_w - p.window_h, [&]() -> int {return p.window_h - getFontH(p.get_font_size(0)*3);});
        SDL_Rect rect = menu_rect;
        menuStack->top()->draw_in_rect(rect);
        }break;
    default:
        break;
    }
    //shade board tile if mouse is hovering over it
    if(getMouseX() < INFO_OFFSET)
    {
        int tileX, tileY;
        GameState::getSelectedTile(p, tileX, tileY);
        fillRect(tileX * TILE_SIZE, tileY * TILE_SIZE, TILE_SIZE, TILE_SIZE, 0, 0, 0, 40);
    }
    //clickables
    for(auto &i: p.clickables[0])
        i->draw(&info_boxes);
    for(auto &i: p.clickables[(int)p.game_view])
        i->draw(&info_boxes);
    //info boxes
    if(p.show_info_boxes)
    {
        if(info_boxes.size() != 0)
        {
            SDL_ShowCursor(false);
            for(auto &i: info_boxes)
                i.draw();
            info_boxes.clear();
        }
    }
    else
    {
        info_boxes.clear();
    }
}
void GameState::operate()
{
    auto &p = players[current_player_turn];
    p.update_input_vars();
    if(getTicksS() >= turn_start_time + turn_length)
    {
        goToNextTurn();
    }
    p.clean_clickables();
    input_events.clear();
    SDL_Event input;
    while(SDL_PollEvent(&input))
    {
        switch(input.type)
        {
        case SDL_QUIT:
            exit(EXIT_SUCCESS);
            break;
        case SDL_MOUSEBUTTONDOWN:
            if(input.button.button == SDL_BUTTON_LEFT)
                handle_mouse_click();
            break;
        case SDL_MOUSEWHEEL:
            break;
        case SDL_KEYDOWN:
            switch(input.key.keysym.sym)
            {
            case SDLK_h:
                flip(p.show_info_boxes);
                break;
            case SDLK_ESCAPE:
                p.action = Player::Action::None;
                break;
            }
            if(p.game_view == Player::GameView::options)
            {
                menuStack->top()->handle_event(input);
            }
            break;
        }
        input_events.push_back(input);
    }
}
void GameState::initNewGame(int num_players)
{
    GameState::reset();
    this->num_players = num_players;
    //reminder: static array is used instead of vector to signal intent to keep the size of the player array constant to prevent
    //iterator invalidation, which would lead to issues
    players = static_array<Player>(num_players);
    for(int i=0; i<num_players; i++)
    {
        auto &p = players[i];
        p.ID = i+1;
        p.game_state = this;
        //view game
        function<int()> x_func = [&p = players[i]]()-> int {return p.window_h;};
        function<int()> y_func = [&p = players[i]]()-> int {return getFontH(p.get_font_size(0));};
        function<int()> s_func = [&p = players[i]]()-> int {return getFontH(p.get_font_size(0));};
        auto button = make_shared<Button>([&p = p]()-> void {p.game_view = Player::GameView::game;},
            nullptr, "View: game", t_game_view, x_func, y_func, s_func, s_func);
        button->should_shade = [&p = players[i]]() -> bool {return p.game_view == Player::GameView::game;};
        p.create_clickable(Player::GameView::global, move(button));
        //view text
        x_func = [&p = players[i]]()-> int {return p.window_h + getFontH(p.get_font_size(0));};
        button = make_shared<Button>([&p = p]()-> void {p.game_view = Player::GameView::text;},
            nullptr, "View: text", t_text_view, x_func, y_func, s_func, s_func);
        button->should_shade = [&p = players[i]]() -> bool {return p.game_view == Player::GameView::text;};
        p.create_clickable(Player::GameView::global, move(button));
        //view options
        x_func = [&p = players[i]]()-> int {return p.window_h + getFontH(p.get_font_size(0)) * 2;};
        button = make_shared<Button>([&p = p]()-> void {p.game_view = Player::GameView::options;},
            nullptr, "View: options", t_options_view, x_func, y_func, s_func, s_func);
        button->should_shade = [&p = players[i]]() -> bool {return p.game_view == Player::GameView::options;};
        p.create_clickable(Player::GameView::global, move(button));
        //end turn
        x_func = [&p = players[i]]()-> int {return p.window_h;};
        y_func = [&p = players[i]]()-> int {return getFontH(p.get_font_size(0)) * 2;};
        p.create_clickable(Player::GameView::global, make_shared<Button>([=]()-> void {goToNextTurn();},
            nullptr, "End turn", t_end_turn, x_func, y_func, s_func, s_func));
        //draw card
        x_func = [&p = players[i]]()-> int {return getWindowH() + getFontH(p.get_font_size(0));};
        p.create_clickable(Player::GameView::game, make_shared<Button>([=]()-> void {drawCard();},
            nullptr, "Draw card", t_draw_card, x_func, y_func, s_func, s_func));
        for(int j=0; j<NUM_RESOURCES; j++)
        {
            function<int()> x_func = [=, &p = players[i]]()-> int {return p.window_h + j * (p.window_w - p.window_h) / NUM_RESOURCES;};
            function<int()> s_func = [&p = players[i]]()-> int {return getFontH(p.get_font_size(0));};
            string desc;
            switch((Resource)j)
            {
            case Resource::Any:
                desc = "Any";
                break;
            case Resource::Air:
                desc = "Air";
                break;
            case Resource::Fire:
                desc = "Fire";
                break;
            case Resource::Earth:
                desc = "Earth";
                break;
            case Resource::Water:
                desc = "Water";
                break;
            case Resource::Aether:
                desc = "Aether";
                break;
            default:
                print_warning("Unknown resource");
                desc = "Unknown Resource";
                break;
            }
            function<void()> exchange_res;
            if(j == 0) //disable exchanging "any" for "any" to prevent the mouse hover shading
                exchange_res = nullptr;
            else exchange_res = [=, &p = players[i]]() -> void {p.exchange_for_any((Resource)j);};
            p.create_clickable(Player::GameView::global, make_shared<Button>(exchange_res, nullptr, desc, t_resource[j], x_func, 0, s_func, s_func));
        }
        for(int j=3; j<=7; j++)
            for(int k=0; k<5; k++)
                p.cards_in_deck.push_back(make_shared<Card>(&p, j));
        for(int j=0; j<10; j++)
        {
            p.cards_in_deck.push_back(make_shared<Card>(&p, 24));
            p.cards_in_deck.push_back(make_shared<Card>(&p, 47));
            p.cards_in_deck.push_back(make_shared<Card>(&p, 1004));
        }
        shuffle(p.cards_in_deck.begin(), p.cards_in_deck.end(), generateRandZ());
    }
    switch(num_players)
    {
    case 2:
        for(int i=0; i<2; i++)
        {
            units.push_back(make_shared<Unit>(2, &players[i], 1 + (BOARD_W-3) * (i%2), 1 + (BOARD_H-3) * (i%2)));
        }
        break;
    default:
        print_error("no known way to initialize game with num_players = " + to_str(num_players));
    }
    player_actions_left = 30;
    turn_length = 60;
    turn_start_time = getTicksS();
}
void GameState::goToNextTurn()
{
    turn_number++;
    current_player_turn++;
    current_player_turn %= players.size();
    auto &p = players[current_player_turn];
    player_actions_left = 30;
    for(auto &i: units)
    {
        if(i->player_owner == &p)
        {
            i->begin_owner_turn(*this);
        }
    }
    auto unit_selected = dynamic_pointer_cast<Unit>(p.selected);
    if(unit_selected != nullptr) //re-add action buttons like move and attack
    {
        //reselect
        select_unit(p, unit_selected);
    }
    turn_start_time = getTicksS();
}
void GameState::drawCard()
{
    if(player_actions_left > 0 && players[current_player_turn].tryToDrawCard())
    {
        player_actions_left--;
    }
}
SDL_Rect GameState::getCardRect(int card_num)
{
    int INFO_OFFSET = getWindowH();
    int INFO_W = getWindowW() - getWindowH();
    constexpr double CARD_BORDER_FRAC = 0.02;
    int CARDS_PER_ROW = 5 - (getWindowH() * 3 > getWindowW() * 2);
    double CARD_W = INFO_W / (double)CARDS_PER_ROW;
    card_num = Player::MAX_CARDS_IN_HAND - card_num - 1;
    card_num += (CARDS_PER_ROW-1) - 2 * (card_num % CARDS_PER_ROW);
    if((Player::MAX_CARDS_IN_HAND / CARDS_PER_ROW) * CARDS_PER_ROW <= card_num)
        card_num -= 2;
    CARD_W = (INFO_W - CARD_W * CARD_BORDER_FRAC * 2) / CARDS_PER_ROW;
    int x = INFO_OFFSET + CARD_W * (CARD_BORDER_FRAC + (card_num % CARDS_PER_ROW));
    int y = getWindowH() - CARD_W * 1.5 * (1 + (card_num / CARDS_PER_ROW));
    int w = CARD_W * (1 - 2 * CARD_BORDER_FRAC);
    int h = w * 1.5;
    x += w * CARD_BORDER_FRAC;
    y += h * CARD_BORDER_FRAC;
    return SDL_Rect{x, y, w, h};
}
