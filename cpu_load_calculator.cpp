#include "cpu_load_calculator.h"
CPULoadCalculator::CPULoadCalculator(ll time_range)
{
    this->time_range = time_range;
    this->cpuTimeSum = 0;
    this->t1 = -NOT_SET; //to help debug
}
void CPULoadCalculator::begin_counting()
{
    ll t = getTicksNs();
    this->t1 = t;
}
void CPULoadCalculator::end_counting()
{
    if(t1 == -NOT_SET)
        print_warning("CPULoadCalculator::end_counting was called before begin_counting");
    ll t2 = getTicksNs();
    cpuTimes.emplace(t1, t2 - t1);
    cpuTimeSum += t2 - t1;
    t1 = -NOT_SET;
}
double CPULoadCalculator::get_load()
{
    ll t = getTicksNs();
    while(!cpuTimes.empty() && cpuTimes.front().A + time_range < t)
    {
        cpuTimeSum -= cpuTimes.front().B;
        cpuTimes.pop();
    }
    if(cpuTimes.size() > 2)
    {
        double div = cpuTimes.back().A + cpuTimes.back().B - cpuTimes.front().A;
        if(div != 0)
            return cpuTimeSum / div;
    }
    return 0;
}
