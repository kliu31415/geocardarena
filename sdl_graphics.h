#pragma once
#include <string>
#include <functional>
#include <vector>
#include <cstddef>
#include <SDL2/SDL.h>
struct VerticalTextDrawer
{
    int x, y, h, fsz, maxx;
    enum class Justify{left, right};
    VerticalTextDrawer(int x, int y, int fsz, int maxx = 0);
    void draw(std::string text = "", Justify justify = Justify::left, uint8_t r=0, uint8_t g=0, uint8_t b=0, uint8_t a=255);
    void drawMultilineUnbroken(std::string text, uint8_t r=0, uint8_t g=0, uint8_t b=0, uint8_t a=255);
    void drawOnSameLine(std::string text, Justify justify = Justify::left, uint8_t r=0, uint8_t g=0, uint8_t b=0, uint8_t a=255);
    void fillRect(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) const;
    void drawRect(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) const;
};

struct InfoBox
{
    std::string text;
    int x, y, w, s, h;
    uint8_t r, g, b, a, tr, tg, tb, ta;
    InfoBox(std::string text, int x, int y, int w, int s, uint8_t r, uint8_t g, uint8_t b, uint8_t a,
                   uint8_t tr=0, uint8_t tg=0, uint8_t tb=0, uint8_t ta=255);
    void draw() const;
};

template<class T> class Expression //small wrapper for function
{
    std::function<T()> expr;
public:
    Expression() = default;
    Expression(T v)
    {
        this->expr = [=]() -> T {return v;};
    }
    template<class Func> Expression(const Func &func)
    {
        this->expr = func;
    }
    T operator()() const
    {
        return expr();
    }
    bool operator == (std::nullptr_t x) const
    {
        return expr == x;
    }
};
using EInt = Expression<int>;
using EBool = Expression<bool>;
using EString = Expression<std::string>;

struct ERect //dynamic rectangle
{
    EInt x, y, w, h;
    ERect() = default;
    ERect(EInt x, EInt y, EInt w, EInt h);
    operator SDL_Rect() const;
};

class Clickable: public ERect
{
protected:
    std::function<void()> func_on_click;
public:
    EBool should_destroy_;
    Clickable() = default;
    Clickable(std::function<void()> func_on_click, std::function<bool()> should_destroy_, EInt x, EInt y, EInt w, EInt h);
    virtual void draw(std::vector<InfoBox> *info_boxes) const;
    bool handle_click() const; //only does stuff if the click is inside the SDL_Rect
    bool should_destroy() const;
};

class Button: public Clickable
{
    std::string desc; //shows the description as an InfoBox if the mouse is hovering over it
    SDL_Texture *t;
public:
    uint8_t texture_alpha = 255;
    bool should_flash = false;
    EBool should_shade = false;
    double border_size = 0; //size as a fraction of the total
    Button() = default;
    Button(std::function<void()> func_on_click, std::function<bool()> should_destroy_, std::string desc, SDL_Texture *t, EInt x, EInt y, EInt w, EInt h);
    virtual void draw(std::vector<InfoBox> *info_boxes) const override;
};

class ButtonWithText: public Button
{
    EString text;
    EInt text_w, font_size;
public:
    ButtonWithText() = default;
    ButtonWithText(EString text, EInt text_w, std::function<void()> func_on_click, std::function<bool()> should_destroy_, std::string desc, SDL_Texture *t, EInt x, EInt y, EInt font_size);
    void draw(std::vector<InfoBox> *info_boxes) const override;
};

struct ScrollableBox
{
    ERect rect;
    double scroll;
    void adjust_scroll(double total_pixels, double pixels);
    ScrollableBox() = default;
    ScrollableBox(EInt x, EInt y, EInt w, EInt h);
};
