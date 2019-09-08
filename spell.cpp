#include "spell.h"
#include "unit.h"
#include "game_state.h"
#include "temporary_mod.h"
#include <memory>
#define SPELL_FUNC_IMPL [](const std::shared_ptr<Spell> &spell) -> void
using namespace std;
Spell::spell_func Spell::BEGIN_ROT_FUNC[Card::NUM_CARDS], Spell::BEGIN_TURN_FUNC[Card::NUM_CARDS], Spell::CAST_FUNC[Card::NUM_CARDS];
Spell::Data::Type Spell::INPUT_DATA_TYPE[Card::NUM_CARDS];
//
Spell::Data::Data(): unit{}
{

}
Spell::Data::Data(Type type, Data &data): unit{} //the shared_ptr inside unit needs to be initialized
{
    switch(type)
    {
    case Data::Type::square:
        square = data.square;
        break;
    case Data::Type::none:
        none = data.none;
        break;
    case Data::Type::unit:
        unit.unit = data.unit.unit;
        break;
    }
}
Spell::Data::~Data()
{

}
//
Spell::Spell(Player *player_owner, int type, Data &input_data):
    Card(player_owner, type),
    input_data(INPUT_DATA_TYPE[type], input_data)
{

}
Spell::~Spell()
{
    switch(INPUT_DATA_TYPE[type])
    {
    case Data::Type::square:
        input_data.square.~Square();
        break;
    case Data::Type::none:
        input_data.none.~None();
        break;
    case Data::Type::unit:
        input_data.unit.~Unit();
        break;
    }
}
void Spell::cast()
{
    CAST_FUNC[type](shared_from_this());
}
void Spell::init(bool load_sprites)
{
    constexpr int Any = (int)Resource::Any, Air = (int)Resource::Air, Water = (int)Resource::Water;
    constexpr int Earth = (int)Resource::Earth, Fire = (int)Resource::Fire, Aether = (int)Resource::Aether;
    using Tmod = TemporaryMod;
    int OFFSET = Unit::NUM_UNITS;
    int cur;
    //
    cur = OFFSET + 2;
    NAME[cur] = "Haste";
    DESCRIPTION[cur] = "The selected unit has +2 speed for one rotation";
    INPUT_DATA_TYPE[cur] = Data::Type::unit;
    COST[cur][Air] = 1;
    CAST_FUNC[cur] = SPELL_FUNC_IMPL
    {
        spell->input_data.unit.unit->add_tmod(Tmod::Affects::UnitSpeed, Tmod::Type::Additive, +2, spell->player_owner->game_state->num_players);
    };
    //
    cur = OFFSET + 4;
    NAME[cur] = "Plan";
    DESCRIPTION[cur] = "Get +1 action this turn. This card takes 0 actions to play.";
    INPUT_DATA_TYPE[cur] = Data::Type::none;
    COST[cur][Any] = 3;
    ACTION_COST[cur] = 0;
    CAST_FUNC[cur] = SPELL_FUNC_IMPL
    {
        spell->player_owner->game_state->player_actions_left += 1;
    };
    //
    cur = OFFSET + 5;
    NAME[cur] = "Ground";
    DESCRIPTION[cur] = "The selected unit has x0 speed for one rotation";
    INPUT_DATA_TYPE[cur] = Data::Type::unit;
    COST[cur][Earth] = 2;
    CAST_FUNC[cur] = SPELL_FUNC_IMPL
    {
        spell->input_data.unit.unit->add_tmod(Tmod::Affects::UnitSpeed, Tmod::Type::Multiplicative, 0, spell->player_owner->game_state->num_players);
    };
    //
    cur = OFFSET + 6;
    NAME[cur] = "Dust storm";
    DESCRIPTION[cur] = "All enemy units have -2 speed";
    INPUT_DATA_TYPE[cur] = Data::Type::none;
    COST[cur][Earth] = 3;
    CAST_FUNC[cur] = SPELL_FUNC_IMPL
    {
        for(auto &i: spell->player_owner->game_state->units)
        {
            if(spell->player_owner != i->player_owner)
                i->add_tmod(Tmod::Affects::UnitSpeed, Tmod::Type::Additive, -2, spell->player_owner->game_state->num_players);
        }
    };
    //
    cur = OFFSET + 16;
    NAME[cur] = "Magic Bolt";
    DESCRIPTION[cur] = "Deals 1 damage to the selected unit. Ignores defense.";
    INPUT_DATA_TYPE[cur] = Data::Type::unit;
    COST[cur][Any] = 2;
    CAST_FUNC[cur] = SPELL_FUNC_IMPL
    {
        spell->player_owner->game_state->unit_receive_damage(spell->input_data.unit.unit.get(), 1);
    };
    //
    if(load_sprites)
    {
        for(int i=OFFSET; i<OFFSET + NUM_SPELLS; i++)
        {
            if(NAME[i] != "")
                SPRITE[i] = loadTexture(("data/spell/" + to_str(i - OFFSET) + ".png").c_str(), 255, 0, 128);
        }
    }
}
