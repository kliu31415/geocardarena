#include "card.h"
#include "unit.h"
#include "game_state.h"
#include <string>
int Card::COST[NUM_CARDS][NUM_RESOURCES];
int Card::ACTION_COST[NUM_CARDS];
SDL_Texture *Card::SPRITE[NUM_CARDS];
std::string Card::NAME[NUM_CARDS], Card::DESCRIPTION[NUM_CARDS];
Card::Card(Player *player_owner, int type)
{
    this->player_owner = player_owner;
    this->type = type;
}
void Card::operate_selected()
{

}
void Card::render_selected(const GameState &game_state, int x, int y, int fsz)
{
    int font_h = getFontH(fsz);
    int origX = x;
    renderCopy(SPRITE[type], x, y, font_h, font_h);
    x += font_h;
    drawText(NAME[type], x, y, fsz);
    x += fsz * 9;
    if(x > getWindowW())
    {
        x = origX;
        y += font_h;
    }
    if(type < Unit::NUM_UNITS)
    {
        renderCopy(t_unit_health, x, y, font_h, font_h);
        if(mouseInRect(x, y, font_h, font_h))
            game_state.info_boxes.emplace_back("Health", getMouseX(), getMouseY(), INF, fsz, 200, 200, 200, 240);
        x += font_h;
        drawText(to_str(Unit::HEALTH[type]), x, y, fsz);
        //play card action
        x = origX;
        y += font_h;
        //stats
        x = origX;
        y += font_h;
        Unit::draw_stat(game_state, Unit::ATTACK[type], t_unit_attack, origX, x, y, fsz);
        Unit::draw_stat(game_state, Unit::DEFENSE[type], t_unit_defense, origX, x, y, fsz);
        Unit::draw_stat(game_state, Unit::RANGE[type], t_unit_range, origX, x, y, fsz);
        Unit::draw_stat(game_state, Unit::SPEED[type], t_unit_speed, origX, x, y, fsz);
        x = origX;
        y += font_h;
    }
    else
    {
        x = origX;
        y += 2*font_h;
    }
    drawMultilineTextUnbroken(DESCRIPTION[type], x, y, getWindowW() - x, fsz);
}
void Card::render(int x, int y, int w, int h) const
{
    if(type < Unit::NUM_UNITS)
        fillRect(x, y, w, h, 200, 190, 230);
    else fillRect(x, y, w, h, 230, 170, 210);
    int text_size = w / 7;
    drawMultilineTextUnbroken(NAME[type], x, y, w, text_size);
    for(int i=0; i<6; i++)
    {
        SDL_Rect dst{x + i * w/6, y + w/3, text_size, text_size};
        setTextureAlphaMod(t_resource[i], 127);
        renderCopy(t_resource[i], &dst);
        setTextureAlphaMod(t_resource[i], 255);
        std::string text = to_str(COST[type][i]);
        if(player_owner->resources[i] < COST[type][i])
            drawText(text, dst.x + w/12.0 - getTextW(text, text_size)/2.0, dst.y, text_size, 255, 0, 0, 150);
        else drawText(text, dst.x + w/12.0 - getTextW(text, text_size)/2.0, dst.y, text_size, 0, 0, 0, 150);
    }
    renderCopy(SPRITE[type], x, y + h - w, w, w);
}
void Card::init()
{
    for(auto &i: ACTION_COST)
        i = 1;
}
