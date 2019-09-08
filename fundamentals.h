#pragma once
#include <iostream>
#include <cassert>
#include <utility>
#include <string>
#include "sdl_base.h"
#define pii std::pair<int, int>
#define A first
#define B second
#define INF ((int)(1e9 + 23000))
#define NOT_SET ((int)(1e9 + 23001))
#define PI 3.14159265358979323
#define TO_DEG(x) (x * 180 / PI)
#define TO_RAD(x) (x * PI / 180)
#define RANDUZ_MAX 0x0fffffff

using ll = long long;
using std::cout;
using std::endl;

template<class T> [[gnu::always_inline]] T sign(T v)
{
    if(v < 0)
        return -1;
    return v > 0;
}

template<class T> [[gnu::always_inline]] T square(T v)
{
    return v*v;
}

inline void flip(bool &x)
{
    x = !x;
}

/**
This prints a string to stdout
*/
void print(std::string s);
/**
This prints a string and appends a newline to stdout
*/
void println(std::string s);
/**
Converts something to a string
*/
template<class T> std::string to_str(T x)
{
    std::stringstream ss;
    ss << x;
    return ss.str();
}
/**
Converts an integer number of seconds to time in the form H:MM:SS
*/
std::string seconds_to_str(int t);
/**
Converts an integer number of seconds to time in the form M:SS or M:SS
*/
std::string seconds_to_str_no_h(int t);
/**
returns either -1 or 1 with 50% probability
*/
int randsign();
/**
Generates a random integer from 0 to RANDUZ_MAX using std::mt19937
*/
int randuz();
/**
Generates random numbers using randuz(). Can be passed to stdlib functions.
*/
struct generateRandZ
{
    typedef size_t result_type;
    static size_t min()
    {
        return 0;
    }
    static size_t max()
    {
        return RANDUZ_MAX;
    }
    size_t operator()()
    {
        return randuz();
    }
};
/**Generates a random integer from 0 to m-1*/
int randuzm(int m);
/**Returns a random float from 0 to 1*/
double randf();
/**Returns a random integer from v1 to v2*/
int randz(int v1, int v2);
/**Rounds a double to the specified number of places (0 to 9)*/
double round(double x, int places);
/**Returns the number of milliseconds since initSDL was called*/
unsigned getTicks();
/**Returns the number of nanoseconds since initSDL was called*/
long long getTicksNs();
/**Returns the number of seconds (as a double) since initSDL was called*/
double getTicksS();
/**Takes in a double and returns a formatted string to the specified number of decimal places*/
std::string format_to_places(double x, int places);

void _print_warning(std::string s, std::string file, int line, std::string func);
void _print_error(std::string s, std::string file, int line, std::string func);
#ifndef NDEBUG
#define print_warning(x) _print_warning(x, __FILE__, __LINE__, __func__)
#define print_error(x) _print_error(x, __FILE__, __LINE__, __func__)
#else
#define print_warning(x) ((void)(0))
#define print_error(x) ((void)(0))
#endif //NDEBUG
