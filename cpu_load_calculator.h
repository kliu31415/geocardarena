#include <queue>
#include <utility>
#include "fundamentals.h"
class CPULoadCalculator
{
    std::queue<std::pair<ll, ll> > cpuTimes; //(time when this was recorded, cpu interval)
    ll cpuTimeSum, time_range, t1;
public:
    CPULoadCalculator(ll time_range);
    void begin_counting();
    void end_counting();
    double get_load();
};
