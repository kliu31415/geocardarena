#pragma once
#include <vector>
struct TemporaryMod
{
    enum class Affects{UnitSpeed, UnitAttack, UnitDefense, UnitRange} affects;
    enum class Type{Additive, Multiplicative, None} type;
    int turns_left;
    int val;
    TemporaryMod(Affects affects, Type type, int val, int time);
    static void get_val(int &val, int &mult, Affects what, std::vector<TemporaryMod> &mods);
    static int get_val(int initial, Affects what, std::vector<TemporaryMod> &mods);
    template<class... Args> static void get_val(int &val, int &mult, Affects what, std::vector<TemporaryMod> &mods, Args &...args)
    {
        get_val(val, mult, what, mods);
        get_val(val, mult, what, args...);
    }
    template<class... Args> static int get_val(int initial, Affects what, std::vector<TemporaryMod> &mods, Args &...args)
    {
        int mult = 1;
        get_val(initial, mult, what, mods);
        get_val(initial, mult, what, args...);
        return initial * mult;
    }
};
