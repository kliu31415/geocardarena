#include "client_chat_box.h"
using namespace std;
SDL_Rect ClientChatBox::get_text_input_rect() const
{
    SDL_Rect pixel_rect = chat_box.rect;
    pixel_rect.y += pixel_rect.h;
    pixel_rect.h = getFontH(font_size());
    return pixel_rect;
}
ClientChatBox::ClientChatBox(EInt x, EInt y, EInt w, EInt h, EInt font_size, uint8_t r, uint8_t g, uint8_t b, uint8_t a):
    chat_box(x, y, w, h)
{
    this->font_size = font_size;
    this->color = SDL_Color{r, g, b, a};
    this->text_input_pos = 0;
}
void ClientChatBox::render()
{
    SDL_Rect pixel_rect = chat_box.rect;
    int fsz = font_size();
    int text_h = getFontH(fsz);
    SDL_Rect text_input_rect = get_text_input_rect();
    fillRect(&text_input_rect, 0, 0, 0, 10);
    fillRect(&pixel_rect, color.r, color.g, color.b, color.a);
    setViewport(&pixel_rect);
    int cur_y = pixel_rect.h + chat_box.scroll;
    total_lines_of_text = 0;
    for(int i=(int)chat_history.size()-1; i>=0; i--)
    {
        int lines = getMultilineTextUnbrokenLength(chat_history[i], pixel_rect.w, fsz);
        total_lines_of_text += lines;
        cur_y -= text_h * lines;
        drawMultilineTextUnbroken(chat_history[i], pixel_rect.x, cur_y, pixel_rect.w, fsz, 0, 0, 0);
    }
    fillRect(pixel_rect.w * 0.97, 0, pixel_rect.w - pixel_rect.w * 0.97, pixel_rect.h, 0, 0, 0, 15);
    setViewport(nullptr);
    if(!SDL_IsTextInputActive() && input_text == "")
    {
        drawText("<click here to type text>", text_input_rect.x, text_input_rect.y, fsz, 0, 0, 0, 100);
    }
    else
    {
        drawText(input_text, text_input_rect.x, text_input_rect.y, fsz);
        if(getTicks() % 1000 < 500)
            fillRect(text_input_rect.x + getTextW(input_text, fsz), text_input_rect.y + text_h * 0.1, text_h * 0.1, text_h * 0.8, 0, 0, 0);
    }
    if(mouseInRect(&pixel_rect))
        fillRect(&pixel_rect, 255, 0, 0, 10);
    if(mouseInRect(&text_input_rect))
        fillRect(&text_input_rect, 255, 0, 0, 10);
}
SDL_Rect ClientChatBox::get_rect()
{
    return (SDL_Rect)chat_box.rect;
}
void ClientChatBox::operate(function<void(string&)> send_chat_func, SDL_Event &input)
{
    SDL_Rect text_input_rect = get_text_input_rect();
    SDL_SetTextInputRect(&text_input_rect);
    switch(input.type)
    {
    case SDL_MOUSEBUTTONDOWN:
        if(input.button.button == SDL_BUTTON_LEFT)
        {
            SDL_Rect rect = get_text_input_rect();
            if(mouseInRect(&rect))
            {
                SDL_StartTextInput();
            }
            else SDL_StopTextInput();
        }
        break;
    case SDL_MOUSEWHEEL:
        chat_box.adjust_scroll(total_lines_of_text * getFontH(font_size()), 0.5 * input.wheel.y * getFontH(font_size()));
        break;
    case SDL_KEYDOWN:
        switch(input.key.keysym.sym)
        {
        case SDLK_ESCAPE:
            SDL_StopTextInput();
            break;
        case SDLK_BACKSPACE: //doesn't work properly on multibyte characters as of right now
            if(!input_text.empty())
            {
                if(SDL_GetModState() & KMOD_CTRL)
                {
                    input_text.pop_back();
                    while(!input_text.empty() && input_text.back() != ' ')
                        input_text.pop_back();
                }
                else input_text.pop_back();
            }
            break;
        case SDLK_RETURN:
        case SDLK_RETURN2:
            send_chat_func(input_text);
            input_text.clear();
            break;
        }
        break;
    case SDL_TEXTINPUT:
        input_text += input.text.text;
        if(input_text.size() > MAX_MESSAGE_LENGTH)
            input_text.resize(MAX_MESSAGE_LENGTH);
        break;
    }
}
void ClientChatBox::add_chat_line(const string &text)
{
    if(text.size() > MAX_MESSAGE_LENGTH + 30)
        chat_history.push_back("received message that is too long");
    else chat_history.push_back(text);
    if(chat_history.size() > MAX_CHAT_LINES)
        chat_history.pop_front();
}
