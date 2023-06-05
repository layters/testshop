#ifndef TIMER_HPP_NEROSHOP
#define TIMER_HPP_NEROSHOP

#include <chrono>

namespace neroshop {
    
class Timer {
public:
    Timer();
    ~Timer();
    
    void start();
    void stop();
    void reset();
    
    double get_elapsed();
    bool get_status() const;

    bool status; // status is 1 if time is being incremented, otherwise returns 0
    double milliseconds;
	double seconds;
private:
    // stopwatch
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time, end_time;
    typedef std::chrono::duration<double, std::ratio<1, 1000>> millisecond_t; // represents duration in milliseconds
    typedef std::chrono::duration<double, std::ratio<1>> second_t;     // represents duration in seconds
};

}

#endif
