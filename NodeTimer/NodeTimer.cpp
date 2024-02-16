// NodeTimer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Timer.h"
#include "FederatedSchedulingTest.h"
#include "FederatedScheduler.h"
#include "FileLock.h"


int main()
{
	//std::cout << (-18 % 7) << std::endl;

	// FederatedSchedulingUnitTestManual();
	std::string TasksStr = std::read_file("TaskSetTest20240216.json");
	nlohmann::json TaskDoc = nlohmann::json::parse(TasksStr);
	STasks tasks{ TaskDoc.template get<STasks>() };

	FederatedScheduler fscheduler(3);
	for (int t = 0; t < tasks.tasks.size(); ++t) fscheduler.add_task(tasks.tasks[t]);
	bool schedulable;
	fscheduler.schedule_task_set(schedulable);
	std::cout << "Task set schedulability: " << schedulable << std::endl;

	std::cout << "Processor assignments:" << std::endl;
	std::vector<ProcessorAssignment> processor_assignments(fscheduler.get_processor_assignments());
	for (int i = 0; i < processor_assignments.size(); ++i) {
		std::cout << "Task ID: " << processor_assignments[i].task_ref->task_detail().task_id << std::endl;
		std::cout << "Active processors: ";
		for (int j = 0; j < processor_assignments[i].active_virtual_processor_refs.size(); ++j)
			std::cout << "{ processor_id: " << processor_assignments[i].active_virtual_processor_refs[j]->processor_id << ", budget: " << processor_assignments[i].active_virtual_processor_refs[j]->budget << " } ";
		std::cout << std::endl;

		std::cout << "Passive processors: ";
		for (int j = 0; j < processor_assignments[i].passive_virtual_processor_refs.size(); ++j)
			std::cout << "{ processor_id: " << processor_assignments[i].passive_virtual_processor_refs[j]->processor_id << ", budget: " << processor_assignments[i].passive_virtual_processor_refs[j]->budget << " } ";
		std::cout << std::endl;
	}



	/*
	STasks tasks{
		std::vector<STask>{
			STask{ -1, 0, -1, 1100, 2000, 1, std::map<int, SJob>{ {1, SJob{ 1, -1, 200, -1, std::vector<int>{2, 3} }}, {2, SJob{ 2, -1, 500, -1, std::vector<int>{4} }}, {3, SJob{ 3, -1, 100, -1, std::vector<int>{4} }}, {4, SJob{ 4, -1, 400, -1, std::vector<int>{} }}  } },
			STask{ -1, 0, -1, 1600, 2000, 2, std::map<int, SJob>{ {1, SJob{ 1, -1, 100, -1, std::vector<int>{2} }}, {2, SJob{ 2, -1, 600, -1, std::vector<int>{3} }}, {3, SJob{ 3, -1, 100, -1, std::vector<int>{} }}  } }
		}
	};

	nlohmann::json TaskDoc;
	to_json(TaskDoc, tasks);

	std::cout << TaskDoc.dump(4) << std::endl;
	std::write_file("TaskSetTest.json", TaskDoc.dump(4));
	*/
	



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
