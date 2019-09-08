#pragma once
#include "card.h"
#include <memory>
#include <cstdint>
class GameState;
class Player;
class Unit;
class Spell: public Card, public std::enable_shared_from_this<Spell>
{
public:
    union Data
    {
        enum class Type{square, none, unit};
        struct Square
        {
            int8_t x, y, h;
        } square;
        struct None
        {

        } none;
        struct Unit //can't be a building
        {
            std::shared_ptr<::Unit> unit;
        } unit;
        Data();
        Data(Type type, Data &data);
        ~Data();
    };
    static constexpr int NUM_SPELLS = 1000;
    typedef void (*spell_func)(const std::shared_ptr<Spell> &spell);
    static spell_func BEGIN_ROT_FUNC[Card::NUM_CARDS], BEGIN_TURN_FUNC[Card::NUM_CARDS], CAST_FUNC[Card::NUM_CARDS];
    static Data::Type INPUT_DATA_TYPE[Card::NUM_CARDS];
    Data input_data;
    Spell(Player *player_owner, int type, Data &input_data);
    ~Spell();
    void cast();
    static void init(bool load_sprites);
};
