#include "timer.hpp"

neroshop::Timer::Timer() : milliseconds(0), seconds(0), status(false)
{}

neroshop::Timer::~Timer() {}

void neroshop::Timer::start()
{
    if(status) {/*std::cout << "Timer is already on." << std::endl;*/ return;}

	status = true; // turn the timer on  
    start_time = std::chrono::high_resolution_clock::now();
}

void neroshop::Timer::stop()
{
    if(!status) {/*std::cout << "Timer is already off." << std::endl;*/ return;}

	end_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = end_time - start_time;
	// save the elapsed time in msecs, secs, mins, and hours
	milliseconds = elapsed.count() * 1000;
	seconds = elapsed.count();
	
	/*std::cout << "It took " << millisecond << " milliseconds to execute.\n";
	std::cout << "It took " << second      << " seconds      to execute.\n";*/
    
    status = false; // turn the timer off
}

void neroshop::Timer::reset() 
{
    milliseconds = 0;
	seconds = 0;
	status = false;
}

bool neroshop::Timer::get_status() const {
    return status;
}

double neroshop::Timer::get_elapsed() {
     // if timer is off, return the time it stopped (end)
    if(!status) { std::chrono::duration<double> elapsed = end_time - start_time; return elapsed.count();}
    // as long as timer is on, get the increments between start and stop
    return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(std::chrono::high_resolution_clock::now() - start_time).count();
}

/*#include <iostream>
int main() {
    neroshop::Timer timer;
    timer.start();
    while(1) {
        std::cout << timer.get_elapsed() << "\n";
    }
}*/ // g++ timer.cpp -o timer
