// NodeTimer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Timer.h"
#include "FederatedScheduler.h"

int main()
{

	/*
		EXAMPLE 2
		Count up an integer and another one down
		every second. If they are at the same value,
		stop the timer from inside.
		This is only possible, when you pass over the
		function with the timer instance reference into
		after initializing the timer instance.
	*/

	/*
	std::vector<SJob> job_map{
		SJob{ 1, -1, 200, -1, std::vector<int>{ 2, 3 }, std::vector<int>{} },
		SJob{ 2, -1, 300, -1, std::vector<int>{ 4 }, std::vector<int>{1} },
		SJob{ 3, -1, 500, -1, std::vector<int>{ 4 }, std::vector<int>{1} },
		SJob{ 4, -1, 200, -1, std::vector<int>{}, std::vector<int>{ 2, 3 } },
	};
	*/

	std::vector<SJob> job_map{
		SJob{ 1, -1, 200, -1, std::vector<int>{ 2, 3, 5 } },
		SJob{ 2, -1, 600, -1, std::vector<int>{ 4 } },
		SJob{ 3, -1, 200, -1, std::vector<int>{ 4 } },
		SJob{ 4, -1, 200, -1, std::vector<int>{ 6 } },
		SJob{ 5, -1, 400, -1, std::vector<int>{ 6 } },
		SJob{ 6, -1, 200, -1, std::vector<int>{} },
	};

	Task test_task(1, 0, 1600);
	test_task.set_job_map(job_map);

	int longest_execution_time;
	std::vector<const SJob* > longest_path = test_task.get_longest_path(longest_execution_time);
	std::cout << "longest path execution time: " << longest_execution_time << std::endl;
	std::cout << "longest path: ";
	for (int i = 0; i < longest_path.size(); ++i) std::cout << longest_path[i]->job_id << " ";
	std::cout << std::endl;

	std::cout << "total execution time: " << test_task.get_total_execution_time() << std::endl;

	FederatedScheduler fscheduler;
 	int processor_demand = fscheduler.compute_processor_demand(test_task);
	std::cout << "processor demand for task 1: " << processor_demand << std::endl;

	
	fscheduler.add_active_processors(std::vector<ActiveVirtualProcessor>{
		ActiveVirtualProcessor(1700, 1),
		ActiveVirtualProcessor(100, 1)
	});
	bool schedulable = fscheduler.is_schedulable_on_active_processors(test_task);
	std::cout << "schedulable on active processors: " << schedulable << std::endl;

	/*
	Timer t2;

	t2.setFunc([&]() {
		std::cout << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
	})
	->setInterval(200)
	->start();

	std::cin.get();
	t2.stop();

	*/

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
