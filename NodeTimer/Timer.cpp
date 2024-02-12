#include "Timer.h"
#include <chrono>

Timer::Timer() {}

Timer::Timer(std::function<void(void)> func, const long& interval) {
	m_func = func;
	m_interval = interval;
}

/**
 * Starting the timer.
 */
void Timer::start() {
	m_running = true;
	m_thread = std::thread([&]() {
		while (m_running) {
			auto delta = std::chrono::steady_clock::now() + std::chrono::milliseconds(m_interval);
			m_func();
			std::this_thread::sleep_until(delta);
		}
		});
	m_thread.detach();
}

/*
 *  Stopping the timer and destroys the thread.
 */
void Timer::stop() {
	m_running = false;
	m_thread.~thread();
}

/*
 *  Restarts the timer. Needed if you set a new
 *  timer interval for example.
 */
void Timer::restart() {
	stop();
	start();
}

/*
 *  Check if timer is running.
 *
 *  @returns boolean is running
 */
bool Timer::isRunning() {
	return m_running;
}


/*
*  Set the method of the timer after
*  initializing the timer instance.
*
*  @returns boolean is running
*  @return  Timer reference of this
*/
Timer* Timer::setFunc(std::function<void(void)> func) {
	m_func = func;
	return this;
}

/*
 *  Returns the current set interval in milliseconds.
 *
 *  @returns long interval
 */
long Timer::getInterval() {
	return m_interval;
}

/*
*  Set a new interval for the timer in milliseconds.
*  This change will be valid only after restarting
*  the timer.
*
*  @param interval new interval
*  @return  Timer reference of this
*/
Timer* Timer::setInterval(const long& interval) {
	m_interval = interval;
	return this;
}

Timer::~Timer() {
	stop();
}