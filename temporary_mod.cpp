#include "temporary_mod.h"
TemporaryMod::TemporaryMod(Affects affects, Type type, int val, int turns)
{
    this->affects = affects;
    this->type = type;
    this->val = val;
    this->turns_left = turns;
}
void TemporaryMod::get_val(int &val, int &mult, Affects what, std::vector<TemporaryMod> &mods)
{
    for(auto &i: mods)
    {
        if(what == i.affects)
        {
            switch(i.type)
            {
            case Type::Additive:
                val += i.val;
                break;
            case Type::Multiplicative:
                mult *= i.val;
                break;
            case Type::None:
                break;
            }
        }
    }
}
int TemporaryMod::get_val(int initial, Affects what, std::vector<TemporaryMod> &mods)
{
    int mult = 1;
    get_val(initial, mult, what, mods);
    return initial * mult;
}
