#pragma once
#include "fundamentals.h"
#include <functional>
#include <deque>
//manages a chat box for a client but doesn't directly connect to a server
class ServerChatBox
{
    int text_input_pos;
    std::deque<std::string> chat_history;
public:
    constexpr static size_t MAX_MESSAGE_LENGTH = 200;
    constexpr static size_t MAX_CHAT_LINES = 1000;
    ServerChatBox() = default;
    void receive_chat_line(const std::string &text);
};
