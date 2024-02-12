// Code copied from https://gist.github.com/zekroTJA/00317b41aa69f38090071b6c8065272b


#include <functional>
#include <thread>

#pragma once
class Timer
{
public:
	Timer();

	Timer(std::function<void(void)> func, const long& interval);

	/**
	 * Starting the timer.
	 */
	void start();

	/*
	 *  Stopping the timer and destroys the thread.
	 */
	void stop();

	/*
	 *  Restarts the timer. Needed if you set a new
	 *  timer interval for example.
	 */
	void restart();

	/*
	 *  Check if timer is running.
	 *
	 *  @returns boolean is running
	 */
	bool isRunning();


	/*
	*  Set the method of the timer after
	*  initializing the timer instance.
	*
	*  @returns boolean is running
	*  @return  Timer reference of this
	*/
	Timer* setFunc(std::function<void(void)> func);

	/*
	 *  Returns the current set interval in milliseconds.
	 *
	 *  @returns long interval
	 */
	long getInterval();

	/*
	*  Set a new interval for the timer in milliseconds.
	*  This change will be valid only after restarting
	*  the timer.
	*
	*  @param interval new interval
	*  @return  Timer reference of this
	*/
	Timer* setInterval(const long& interval);

	~Timer();

private:
	// Function to be executed fater interval
	std::function<void(void)> m_func;
	// Timer interval in milliseconds
	long m_interval;

	// Thread timer is running into
	std::thread m_thread;
	// Status if timer is running
	bool m_running = false;
};

