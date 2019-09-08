#include "fundamentals.h"
#include "sdl_base.h"
#include <string>
#include <cstdio>
#include <chrono>
#include <random>
#include <cmath>
#include <mutex>
static std::string clipFilePath(std::string &path)
{
    for(int i=path.size()-1; i>=0; i--)
        if(path[i]=='\\' || path[i]=='/')
            return path.substr(i+1);
    return path;
}
void _print_warning(std::string s, std::string file, int line, std::string func)
{
    println("Warning (file=" + clipFilePath(file) + ", line=" + to_str(line) + ", func=" + to_str(func) + "): " + s);
}
void _print_error(std::string s, std::string file, int line, std::string func)
{
    println("Error (file=" + clipFilePath(file) + ", line=" + to_str(line) + ", func=" + to_str(func) + "): " + s);
}
std::mutex stdoutMutex;
void print(std::string s)
{
    std::lock_guard<std::mutex> lock(stdoutMutex);
    std::puts(("[" + seconds_to_str(getTicksS()) + "]" + s).c_str());
    std::fflush(stdout);
}
void println(std::string s)
{
    std::lock_guard<std::mutex> lock(stdoutMutex);
    std::puts(("[" + seconds_to_str(getTicksS()) + "]" + s).c_str());
    std::fflush(stdout);
}
std::string seconds_to_str(int t)
{
    int s = t%60;
    t /= 60;
    int m = t%60;
    int h = t / 60;
    std::string res;
    if(h == 0)
        res += "0";
    else res += to_str(h);
    res += ":";
    if(m == 0)
        res += "00";
    else if(m < 10)
    {
        res += "0";
        res += to_str(m);
    }
    else res += to_str(m);
    res += ":";
    if(s == 0)
        res += "00";
    else if(s < 10)
    {
        res += "0";
        res += to_str(s);
    }
    else res += to_str(s);
    return res;
}
std::string seconds_to_str_no_h(int t)
{
    int s = t%60;
    t /= 60;
    int m = t%60;
    std::string res;
    res += to_str(m);
    res += ":";
    if(s == 0)
        res += "00";
    else if(s < 10)
    {
        res += "0";
        res += to_str(s);
    }
    else res += to_str(s);
    return res;
}
int randsign()
{
    return 1 - 2 * randz(0, 1);
}
int randuz()
{
    static std::mt19937 gen(time(NULL));
    static std::uniform_int_distribution<int> d(0, RANDUZ_MAX);
    return d(gen);
}
int randuzm(int m)
{
    return randuz() % m;
}
double randf()
{
    return randuz() / (double)RANDUZ_MAX;
}
int randz(int v1, int v2)
{
    double diff = v2 - v1 + 0.99999999;
    return randf()*diff + v1;
}
double round(double x, int places)
{
    double p = pow(10, places);
    if(x >= 0)
        return std::floor(x * p + 0.5) / p;
    else return std::floor(x * p - 0.5) / p;
}
unsigned getTicks()
{
    return getTicksNs() / 1000000;
}
long long getTicksNs()
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto t = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(t - startTime).count();
}
double getTicksS()
{
    return getTicksNs() / 1e9;
}
std::string format_to_places(double x, int places)
{
    std::string res = to_str(round(x, places));
    for(size_t i=0; i<res.size(); i++)
    {
        if(res[i] == '.')
        {
            while(res.size() < i + places + 1)
                res += '0';
            return res;
        }
    }
    res += '.';
    for(int i=0; i<places; i++)
        res += '0';
    return res;
}
