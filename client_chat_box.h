#pragma once
#include "fundamentals.h"
#include "sdl_graphics.h"
#include <functional>
#include <deque>
//manages a chat box for a client but doesn't directly connect to a server (another class handles that)
class ClientChatBox
{
    SDL_Color color;
    EInt font_size;
    int text_input_pos;
    std::string input_text;
    SDL_Rect get_text_input_rect() const;
    std::deque<std::string> chat_history;
    ScrollableBox chat_box;
    int total_lines_of_text = 0;
public:
    constexpr static size_t MAX_MESSAGE_LENGTH = 150;
    constexpr static size_t MAX_CHAT_LINES = 50;
    ClientChatBox() = default;
    ClientChatBox(EInt x, EInt y, EInt w, EInt h, EInt font_size, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void render();
    SDL_Rect get_rect();
    void operate(std::function<void(std::string&)> send_chat_func, SDL_Event &input);
    void add_chat_line(const std::string &text);
};
