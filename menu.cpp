#include "menu.h"
#include "fundamentals.h"
#include "basic_game_funcs.h"
#include "sdl_graphics.h"
#include "game_state.h"
#include "lobby_client.h"
std::shared_ptr<StandardMenu> main_menu, program_info_menu, options_menu, game_options_menu, cpugpu_menu, death_menu, play_menu, login_menu;
std::shared_ptr<std::stack<std::shared_ptr<Menu> > > menuStack;
namespace in_game_menu
{
    std::shared_ptr<StandardMenu> main;
}

void addToMenuStack(std::shared_ptr<Menu> menu)
{
    menuStack->push(menu);
    menu->reset();
}
MenuOption::MenuOption(std::string text, std::string description, void (*funcOnPress)(InputType), std::string (*displayVar)())
{
    this->text = text;
    this->description = description;
    this->funcOnPress = funcOnPress;
    this->displayVar = displayVar;
}
StandardMenu::StandardMenu(std::string title)
{
    this->title = title;
    textSizeMult = 1;
    widthF = 0.6; //fraction of the screen, NOT a raw value in pixels
    curOption = 0;
    customDrawingFunc = nullptr;
}
void StandardMenu::handle_event(SDL_Event &input)
{
    switch(input.type)
    {
    case SDL_QUIT:
        exit(EXIT_SUCCESS);
        break;
    case SDL_KEYDOWN:
        switch(input.key.keysym.sym)
        {
        case SDLK_RETURN:
        case SDLK_RETURN2:
            if(options[curOption].funcOnPress != nullptr)
                options[curOption].funcOnPress(MenuOption::InputType::enter);
            break;
        case SDLK_UP:
            curOption = (curOption + options.size() - 1) % options.size();
            break;
        case SDLK_DOWN:
            curOption = (curOption + 1) % options.size();
            break;
        case SDLK_LEFT:
            if(options[curOption].funcOnPress != nullptr)
                options[curOption].funcOnPress(MenuOption::InputType::left);
            break;
        case SDLK_RIGHT:
            if(options[curOption].funcOnPress != nullptr)
                options[curOption].funcOnPress(MenuOption::InputType::right);
            break;
        case SDLK_ESCAPE:
            if(menuStack->size() > 1)
                menuStack->pop();
            break;
        }
    }
    if(options[curOption].captureInput != nullptr)
        options[curOption].captureInput(options[curOption], input);
}
void StandardMenu::operate()
{
    if(options[curOption].captureInput != nullptr)
    {
        SDL_StartTextInput();
    }
    else if(SDL_IsTextInputActive())
        SDL_StopTextInput();
    SDL_Event input;
    while(SDL_PollEvent(&input))
    {
        handle_event(input);
    }
}
void StandardMenu::draw(uint8_t alpha /*=255*/, bool clearScreen /*=true*/) //CODE_WARNING: drawing the "program info" menu resulted in the program not responding once. Unsure of how to reproduce.
{
    if(clearScreen)
        renderClear(200, 200, 200, alpha);
    int fsz = getFontSize(0.5);
    int text_h = getFontH(fsz);
    int borderSize = text_h / 2;
    VerticalTextDrawer menuText((1 - widthF)/2 * getWindowW(),
            (getWindowH() - text_h * (int)options.size())/2, fsz, (1 + widthF)/2 * getWindowW());
    drawText(title, menuText.x, menuText.y - borderSize - text_h, fsz, 0, 0, 0, alpha);
    fillRect(menuText.x - borderSize, menuText.y - borderSize, menuText.maxx - menuText.x + borderSize * 2,
             text_h * (int)options.size() + borderSize * 2, 170, 170, 170, alpha);
    fillRect(menuText.x, menuText.y, menuText.maxx - menuText.x, text_h * (int)options.size(), 220, 220, 220, alpha);
    for(int i=0; i<(int)options.size(); i++)
    {
        if(i == curOption)
            menuText.fillRect(255, 0, 0, (50/255.0) * alpha);
        if(options[i].captureInput != nullptr)
        {
            menuText.draw(options[i].text + ": " + options[i].input_text, VerticalTextDrawer::Justify::left, 0, 100, 0);
        }
        else menuText.draw(options[i].text, VerticalTextDrawer::Justify::left, 0, 0, 0, alpha);
        if(options[i].displayVar != nullptr)
            menuText.drawOnSameLine(options[i].displayVar(), VerticalTextDrawer::Justify::right, 0, 100, 0, alpha);
    }
    menuText.y += borderSize;
    if(customDrawingFunc != nullptr)
        customDrawingFunc(menuText.y); //doesn't work with the alpha value
    menuText.h *= 0.7; //the description should be in a bit smaller font
    menuText.drawMultilineUnbroken(options[curOption].description, 0, 0, 0, alpha);
}
void StandardMenu::draw_in_rect(SDL_Rect &rect)
{
    setViewport(&rect);
    int fsz = getFontSize(0);
    int text_h = getFontH(fsz);
    VerticalTextDrawer menuText(0, 0, fsz, rect.w);
    menuText.draw(title);
    fillRect(0, menuText.y, menuText.maxx, text_h / 2, 0, 0, 0, 100);
    menuText.y += text_h / 2;
    for(int i=0; i<(int)options.size(); i++)
    {
        if(i == curOption)
            menuText.fillRect(255, 0, 0, 50);
        menuText.draw(options[i].text, VerticalTextDrawer::Justify::left, 0, 0, 0);
        if(options[i].displayVar != nullptr)
            menuText.drawOnSameLine(options[i].displayVar(), VerticalTextDrawer::Justify::right, 0, 100, 0);
    }
    fillRect(0, menuText.y, menuText.maxx, text_h / 2, 0, 0, 0, 100);
    menuText.y += text_h / 2;
    if(customDrawingFunc != nullptr)
        customDrawingFunc(menuText.y); //doesn't work with the alpha value
    menuText.h *= 0.7; //the description should be in a bit smaller font
    menuText.drawMultilineUnbroken(options[curOption].description, 0, 0, 0);
    setViewport(nullptr);
}
void StandardMenu::add_back_button()
{
    options.emplace_back("Back", "",
        [](MenuOption::InputType t )-> void {
            if(t == MenuOption::InputType::enter)
            {
                menuStack->pop();
            }});
}
namespace ProgramInfo
{
    std::string compiler, OS, sdl_version, game_version;
    int bits;
    void init()
    {
        //compiler
        #if defined(__ICC) || defined(__ICL)
        compiler = "Intel C++";
        #elif __GNUC__
        compiler = "GCC ";
        compiler += to_str(__GNUC__) + "." + to_str(__GNUC_MINOR__) + "." + to_str(__GNUC_PATCHLEVEL__);
        #elif _MSC_VER
        compiler = "MSVC " + to_str((int)_MSC_VER);
        #elif __clang__
        compiler = "clang " + __clang_version__;
        #else
        compiler = "unknown";
        #endif
        //OS
        #if defined(_WIN64) || defined(_WIN32)
        OS = "Windows";
        #elif __linux__
        OS = "Linux";
        #elif TARGET_OS_MAC
        OS = "Apple Mac";
        #else
        OS = "unknown";
        #endif // _WIN64
        //get SDL2 version
        SDL_version v;
        SDL_VERSION(&v);
        sdl_version = to_str((int)v.major) + "." + to_str((int)v.minor) + "." + to_str((int)v.patch);
        game_version = to_str(GAME_VERSION_MAJOR) + "." + to_str(GAME_VERSION_MINOR) + "." + to_str(GAME_VERSION_PATCH) + " " + GAME_VERSION_DESC;
    }
    void display(int y)
    {
        int fsz = getFontSize(0);
        VerticalTextDrawer text(0, y, fsz, getWindowW());
        text.draw("Creator: assert0");
        text.draw("Game name: GeoCardArena v" + game_version);
        text.draw("Build time: " + to_str(__DATE__) + " " + to_str(__TIME__));
        text.draw("Compiler: " + compiler);
        text.draw("Operating System: " + OS);
        text.draw("Graphics: SDL2 v" + sdl_version + " (zlib License)");
        text.draw("Credits: see credits.txt");
    }
}
void StandardMenu::reset()
{
    curOption = 0;
}
void StandardMenu::init()
{
    //
    main_menu = std::make_shared<StandardMenu>("Main Menu");
    main_menu->options.emplace_back("Play", "Local/Online",
        [](MenuOption::InputType t) -> void {
            if(t == MenuOption::InputType::enter)
            {
                addToMenuStack(play_menu);
            }});
    main_menu->options.emplace_back("Options", "",
        [](MenuOption::InputType t) -> void {
            if(t == MenuOption::InputType::enter)
            {
                addToMenuStack(options_menu);
            }});
    main_menu->options.emplace_back("Visit", "You should give feedback by telling me what's unbalanced/boring",
        [](MenuOption::InputType t) -> void {
            if(t == MenuOption::InputType::enter)
            {
                #ifdef _WIN32
                system("explorer www.assertfalse.wordpress.com");
                #elif __linux__
                system("sensible-browser assertfalse.wordpress.com");
                #endif
            }});
    main_menu->options.emplace_back("Program Info", "Displays program info",
        [](MenuOption::InputType t) -> void {
            if(t == MenuOption::InputType::enter)
            {
                addToMenuStack(program_info_menu);
            }});
    main_menu->options.emplace_back("Exit", "Self-explanatory (I hope)",
        [](MenuOption::InputType t) -> void {
            if(t == MenuOption::InputType::enter)
            {
                exit(EXIT_SUCCESS);
            }});
    //
    play_menu = std::make_shared<StandardMenu>("Play");
    play_menu->options.emplace_back("Local 2P", "",
        [](MenuOption::InputType t) -> void {
            if(t == MenuOption::InputType::enter)
            {
                current_global_game_state = global_game_state::in_game;
                *menuStack = std::stack<std::shared_ptr<Menu> >();
                game_state = std::make_shared<GameState>(2);
            }});
    play_menu->options.emplace_back("Local 3P", "Doesn't work",
        [](MenuOption::InputType t) -> void {
            if(t == MenuOption::InputType::enter)
            {

            }});
    play_menu->options.emplace_back("Local 4P", "Doesn't work",
        [](MenuOption::InputType t) -> void {
            if(t == MenuOption::InputType::enter)
            {

            }});
    play_menu->options.emplace_back("Online", "Doesn't work",
        [](MenuOption::InputType t) -> void {
            if(t == MenuOption::InputType::enter)
            {
                addToMenuStack(login_menu);
            }});
    play_menu->add_back_button();
    //
    login_menu = std::make_shared<StandardMenu>("Online");
    login_menu->options.emplace_back("Display Name", "Doesn't do anything right now", nullptr);
    login_menu->options.back().captureInput =
        [](MenuOption &menu_option, SDL_Event &input) -> void {
                switch(input.type)
                {
                case SDL_TEXTINPUT:
                    menu_option.input_text += input.text.text;
                    if(menu_option.input_text.size() > 15)
                        menu_option.input_text.resize(15);
                    break;
                case SDL_KEYDOWN:
                    if(input.key.keysym.sym == SDLK_BACKSPACE)
                    {
                        if(!menu_option.input_text.empty())
                            menu_option.input_text.pop_back();
                    }
                    break;
                }
            };
    login_menu->options.emplace_back("Join Lobby", "",
        [](MenuOption::InputType t) -> void {
            if(t == MenuOption::InputType::enter)
            {
                lobby_client->connect();
                current_global_game_state = global_game_state::online_lobby;
                *menuStack = std::stack<std::shared_ptr<Menu> >();
            }
            });
    login_menu->customDrawingFunc = [](int y) -> void
    {
        std::string text;
        /*y += getFontH(getFontSize(0.5));
        if(!lobby_client->client.connection.is_connected()) //it's bad to put this in the drawing function but whatever
            text = "Disconnected";
        else
        {
            lobby_client->operate();
            if(lobby_client->public_data == "good")
            {
                current_global_game_state = global_game_state::online_lobby;
                *menuStack = std::stack<std::shared_ptr<Menu> >();
            }
            else
            {
                text = lobby_client->public_data;
            }
        }*/
        drawText(text, 0.2 * getWindowW(), y, getFontSize(0.5), 100, 0, 0);
    };
    login_menu->add_back_button();
    //
    program_info_menu = std::make_shared<StandardMenu>("Program Info");
    ProgramInfo::init();
    program_info_menu->customDrawingFunc = ProgramInfo::display;
    program_info_menu->add_back_button();
    //
    options_menu = std::make_shared<StandardMenu>("Options");
    options_menu->options.emplace_back("Game Options", "Customize the game!",
        [](MenuOption::InputType t )-> void {
            if(t == MenuOption::InputType::enter)
            {
                addToMenuStack(game_options_menu);
            }});
    options_menu->options.emplace_back("CPU/Graphics/Music", "Miscellaneous options",
        [](MenuOption::InputType t )-> void {
            if(t == MenuOption::InputType::enter)
            {
                addToMenuStack(cpugpu_menu);
            }});
    options_menu->add_back_button();
    //
    game_options_menu = std::make_shared<StandardMenu>("Game Options");
    game_options_menu->add_back_button();
    //
    cpugpu_menu = std::make_shared<StandardMenu>("CPU/Graphics/Music");
    cpugpu_menu->options.emplace_back("Show Extra Info", "Shows the program's utilization fraction of a CPU core (and possibly other things)",
        [](MenuOption::InputType t) -> void {
            switch(t)
            {
            case MenuOption::InputType::left:
            case MenuOption::InputType::right:
            case MenuOption::InputType::enter:
                SHOW_EXTRA_INFO = !SHOW_EXTRA_INFO;
                break;
            default:
                break;
            }}, []()-> std::string {
            if(SHOW_EXTRA_INFO)
                return "Yah";
            else return "Nah";
            }
            );
    cpugpu_menu->options.emplace_back("Text Size", "Increasing this value makes some text bigger. Making this value too extreme may result in graphical glitches.",
        [](MenuOption::InputType t) -> void {
            switch(t)
            {
            case MenuOption::InputType::left:
                sdl_settings::textSizeMult = std::max(0.1, sdl_settings::textSizeMult - 0.1);
                break;
            case MenuOption::InputType::right:
                sdl_settings::textSizeMult = std::min(4.0, sdl_settings::textSizeMult + 0.1);
                break;
            default:
                break;
            }}, []()-> std::string {
            return to_str(sdl_settings::textSizeMult);
            }
            );
    cpugpu_menu->options.emplace_back("Vsync", "Vertical Sync caps your FPS at your screen refresh rate. Requires restarting the program.",
        [](MenuOption::InputType t) -> void {
            switch(t)
            {
            case MenuOption::InputType::left:
            case MenuOption::InputType::right:
            case MenuOption::InputType::enter:
                sdl_settings::vsync = !sdl_settings::vsync;
                break;
            default:
                break;
            }}, []()-> std::string {
            if(sdl_settings::vsync)
                return "Yes";
            else return "No";
            }
            );
    cpugpu_menu->options.emplace_back("Fullscreen", "Choose whether to go fullscreen or not. Requires restarting the program. Note that "
                                      "the window can be resized in non-fullscreen mode",
        [](MenuOption::InputType t) -> void {
            switch(t)
            {
            case MenuOption::InputType::left:
            case MenuOption::InputType::right:
            case MenuOption::InputType::enter:
                if(sdl_settings::IS_FULLSCREEN)
                {
                    sdl_settings::WINDOW_W = 1e9 + 1;
                    sdl_settings::WINDOW_H = 1e9 + 1;
                }
                sdl_settings::IS_FULLSCREEN = !sdl_settings::IS_FULLSCREEN;
                break;
            default:
                break;
            }}, []()-> std::string {
            if(sdl_settings::IS_FULLSCREEN)
                return "Yes";
            else return "No";
            }
            );
    cpugpu_menu->options.emplace_back("SFX Volume", "Note that the max volume is not necessarily 100",
        [](MenuOption::InputType t) -> void {
            switch(t)
            {
            case MenuOption::InputType::left:
                setSfxVolume(sdl_settings::sfxVolume - 1);
                break;
            case MenuOption::InputType::right:
                setSfxVolume(sdl_settings::sfxVolume + 1);
                break;
            default:
                break;
            }}, []()-> std::string {
            return to_str(sdl_settings::sfxVolume);
            }
            );
    cpugpu_menu->options.emplace_back("Music Volume", "Note that the max volume is not necessarily 100",
        [](MenuOption::InputType t) -> void {
            switch(t)
            {
            case MenuOption::InputType::left:
                setMusicVolume(sdl_settings::musicVolume - 1);
                break;
            case MenuOption::InputType::right:
                setMusicVolume(sdl_settings::musicVolume + 1);
                break;
            default:
                break;
            }}, []()-> std::string {
            return to_str(sdl_settings::musicVolume);
            }
            );
    cpugpu_menu->add_back_button();
    //
    death_menu = std::make_shared<StandardMenu>("You died");
    death_menu->options.emplace_back("Back to Main Menu", "You died... go back to the main menu to restart"
                                    /*"(if passed floor 2)You feel calmness as your consciousness drifts away... but you get a strange feeling "
                                    "that the end is still not in sight"*/,
        [](MenuOption::InputType t)-> void {
            if(t == MenuOption::InputType::enter)
            {
                current_global_game_state = global_game_state::menu;
                *menuStack = std::stack<std::shared_ptr<Menu> >();
                addToMenuStack(main_menu);

            }});
    //
    in_game_menu::main = std::make_shared<StandardMenu>("In-game menu");
    in_game_menu::main->options.emplace_back("CPU/Graphics/Music", "",
        [](MenuOption::InputType t )-> void {
            if(t == MenuOption::InputType::enter)
            {
                addToMenuStack(cpugpu_menu);
            }});
    in_game_menu::main->options.emplace_back("Program Info", "",
        [](MenuOption::InputType t )-> void {
            if(t == MenuOption::InputType::enter)
            {
                addToMenuStack(program_info_menu);
            }});
    in_game_menu::main->options.emplace_back("Quit to Main Menu", "Progress will not be saved",
        [](MenuOption::InputType t) -> void {
            if(t == MenuOption::InputType::enter)
            {
                current_global_game_state = global_game_state::menu;
                *menuStack = std::stack<std::shared_ptr<Menu> >();
                menuStack = global_menu_stack;
                addToMenuStack(main_menu);
                //when the menuStack is empty the pause menu is over. This is detected in the operatePauseMenu function
            }});
}
