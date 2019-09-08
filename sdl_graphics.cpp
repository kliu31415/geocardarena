#include "sdl_graphics.h"
#include "sdl_base.h"
#include "fundamentals.h"
#include <cmath>
//VerticalTextDrawer
VerticalTextDrawer::VerticalTextDrawer(int x, int y, int fsz, int maxx)
{
    this->x = x;
    this->y = y;
    this->fsz = fsz;
    this->h = getFontH(fsz);
    this->maxx = maxx;
}
void VerticalTextDrawer::draw(std::string text, Justify justify, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    switch(justify)
    {
    case Justify::right:
        drawText(text, maxx - getTextW(text, fsz), y, fsz, r, g, b, a);
        break;
    case Justify::left:
        drawText(text, x, y, fsz, r, g, b, a);
        break;
    }
    y += h;
}
void VerticalTextDrawer::drawMultilineUnbroken(std::string text, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    y += h * drawMultilineTextUnbroken(text, x, y, maxx - x, fsz, r, g, b, a);
}
void VerticalTextDrawer::drawOnSameLine(std::string text, Justify justify, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    y -= h;
    draw(text, justify, r, g, b, a);
}
void VerticalTextDrawer::fillRect(uint8_t r, uint8_t g, uint8_t b, uint8_t a) const
{
    ::fillRect(x, y, maxx-x, h, r, g, b, a);
}
void VerticalTextDrawer::drawRect(uint8_t r, uint8_t g, uint8_t b, uint8_t a) const
{
    ::drawRect(x, y, maxx-x, h, r, g, b, a);
}

//InfoBox
InfoBox::InfoBox(std::string text, int x, int y, int w, int s, uint8_t r, uint8_t g, uint8_t b, uint8_t a,
                   uint8_t tr, uint8_t tg, uint8_t tb, uint8_t ta)
{
    this->text = text;
    this->x = x;
    this->y = y;
    this->w = w;
    this->s = s;
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
    this->tr = tr;
    this->tg = tg;
    this->tb = tb;
    this->ta = ta;
    if(this->w == INF)
    {
        this->h = this->s;
        getTextSize(this->text, this->s, &this->w, nullptr);
    }
    else this->h = getMultilineTextUnbrokenLength(this->text, this->w, this->s);
}
void InfoBox::draw() const
{
    if(w == INF)
    {
        fillRect(x, y, w, getFontH(s), r, g, b, a);
        drawText(text, x, y, s, tr, tg, tb, ta);
    }
    else
    {
        fillRect(x, y, w, getFontH(s), r, g, b, a);
        drawMultilineTextUnbroken(text, x, y, w, s, tr, tg, tb, ta);
    }
}

//ERect
ERect::ERect(EInt x, EInt y, EInt w, EInt h): x(x), y(y), w(w), h(h)
{

}
ERect::operator SDL_Rect() const
{
    return SDL_Rect{x(), y(), w(), h()};
}

//Clickable
Clickable::Clickable(std::function<void()> func_on_click, std::function<bool()> should_destroy_, EInt x, EInt y, EInt w, EInt h): ERect(x, y, w, h)
{
    this->func_on_click = func_on_click;
    this->should_destroy_ = should_destroy_;
}
void Clickable::draw(std::vector<InfoBox> *info_boxes) const
{

}
bool Clickable::handle_click() const
{
    SDL_Rect dst{x(), y(), w(), h()};
    if(func_on_click != nullptr)
    {
        if(mouseInRect(&dst))
        {
            func_on_click();
            return true;
        }
    }
    return false;
}
bool Clickable::should_destroy() const
{
    if(should_destroy_ == nullptr)
        return false;
    return should_destroy_();
}
//Button
Button::Button(std::function<void()> func_on_click, std::function<bool()> should_destroy_, std::string desc, SDL_Texture *t, EInt x, EInt y, EInt w, EInt h): Clickable(func_on_click, should_destroy_, x, y, w, h)
{
    this->desc = desc;
    this->t = t;
}
void Button::draw(std::vector<InfoBox> *info_boxes) const
{
    SDL_Rect dst{x(), y(), w(), h()};
    if(mouseInRect(&dst))
    {
        if(func_on_click != nullptr)
            fillRect(&dst, 0, 0, 0, 30);
        if(desc != "" && info_boxes != nullptr)
            info_boxes->emplace_back(desc, getMouseX(), getMouseY(), INF, getFontSize(0), 200, 200, 200, 240);
    }
    if(border_size != 0)
    {
        setColor(0, 0, 0, 100);
        fillRect(dst.x, dst.y, dst.w, dst.h * border_size);
        fillRect(dst.x, dst.y + dst.h * border_size, dst.w * border_size, dst.h*(1 - 2*border_size));
        fillRect(dst.x + dst.w * (1 - border_size), dst.y + dst.h * border_size, (int)dst.w - (int)(dst.w * (1 - border_size)), dst.h*(1 - 2*border_size));
        fillRect(dst.x, dst.y + dst.h * (1 - border_size), dst.w, dst.h * border_size);
        dst.x += dst.w * border_size;
        dst.y += dst.h * border_size;
        dst.w *= 1 - 2 * border_size;
        dst.h *= 1 - 2 * border_size;
    }
    SDL_SetTextureAlphaMod(t, texture_alpha);
    renderCopy(t, &dst);
    SDL_SetTextureAlphaMod(t, 255);
    if(should_flash)
    {
        fillRect(&dst, 0, 0, 0, 40 + 40 * std::sin(2 * PI * getTicksS()));
    }
    if(should_shade())
    {
        fillRect(&dst, 0, 0, 0, 60);
    }
}
//ButtonWithText
ButtonWithText::ButtonWithText(EString text, EInt text_w, std::function<void()> func_on_click, std::function<bool()> should_destroy_, std::string desc, SDL_Texture *t, EInt x, EInt y, EInt font_size):
    Button(func_on_click, should_destroy_, desc, t, x, y, [&]() -> int {return getFontH(font_size());}, [&]() -> int {return getFontH(font_size());})
{
    this->text = text;
    this->text_w = text_w;
    this->font_size = font_size;
}
void ButtonWithText::draw(std::vector<InfoBox> *info_boxes) const
{
    Button::draw(info_boxes);
    drawMultilineTextUnbrokenStart(text(), x(), y(), text_w(), font_size(), getFontH(font_size()));
}
//ScrollableBox
ScrollableBox::ScrollableBox(EInt x, EInt y, EInt w, EInt h)
{
    this->rect = ERect{x, y, w, h};
    this->scroll = 0;
}
void ScrollableBox::adjust_scroll(double total_pixels, double pixels)
{
    scroll = std::max(0.0, std::min(total_pixels - rect.h(), scroll + pixels));
}
