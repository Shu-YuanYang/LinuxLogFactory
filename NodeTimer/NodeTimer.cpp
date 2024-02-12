// NodeTimer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Timer.h"

int main()
{
    std::cout << "Hello World!\n";

	/*
		EXAMPLE 1
		Initialize a timer which will count up
		the integer every second and output the
		result.
	*/

	/*int countMeUp = 0;

	Timer t1([&]() {
		countMeUp++;
		std::cout << countMeUp << std::endl;
		}, 1000);

	t1.start();

	system("pause");*/

	/*
		EXAMPLE 2
		Count up an integer and another one down
		every second. If they are at the same value,
		stop the timer from inside.
		This is only possible, when you pass over the
		function with the timer instance reference into
		after initializing the timer instance.
	*/

	int iGoUp = 0, iGoDown = 10;
	Timer t2;

	t2.setFunc([&]() {
		std::cout << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
	})
	->setInterval(200)
	->start();

	system("pause");

	t2.stop();


	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
