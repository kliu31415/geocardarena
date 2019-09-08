#include "server_chat_box.h"
using namespace std;
void ServerChatBox::receive_chat_line(const string &text)
{
    //add this to the chat history
    chat_history.push_back(text);
    if(chat_history.size() > MAX_CHAT_LINES)
        chat_history.pop_front();
    //send it
}
