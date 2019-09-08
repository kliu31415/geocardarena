#pragma once
#include "basic_game_funcs.h"
#include <string>
#include <vector>
#include <stack>
#include <memory>

struct MenuOption
{
    enum class InputType{left, right, enter};
    void (*funcOnPress)(InputType);
    void (*captureInput)(MenuOption &menu_option, SDL_Event &input) = nullptr; //usually text input
    std::string (*displayVar)();
    std::string text, description, input_text;
    MenuOption(std::string text, std::string description, void (*funcOnPress)(InputType) = nullptr, std::string (*displayVar)() = nullptr);
};

class SDL_Rect;

class Menu
{
public:
    virtual void reset() = 0;
    virtual void handle_event(SDL_Event &input) = 0;
    virtual void operate() = 0;
    virtual void draw(uint8_t alpha = 255, bool clearScreen = true) = 0;
    virtual void draw_in_rect(SDL_Rect &rect) = 0;
};

class StandardMenu: public Menu
{
    double textSizeMult, widthF;
    std::string title;
    std::vector<MenuOption> options;
    void (*customDrawingFunc)(int y);
    void add_back_button();
public:
    int curOption;
    StandardMenu(std::string title);
    static void init();
    void handle_event(SDL_Event &input) override;
    void operate() override;
    void draw(uint8_t alpha = 255, bool clearScreen = true) override;
    void draw_in_rect(SDL_Rect &rect) override;
    void reset() override;
};


void addToMenuStack(std::shared_ptr<Menu> menu);
extern std::shared_ptr<std::stack<std::shared_ptr<Menu> > > menuStack;
extern std::shared_ptr<StandardMenu> main_menu, death_menu;
namespace in_game_menu
{
    extern std::shared_ptr<StandardMenu> main;
}
