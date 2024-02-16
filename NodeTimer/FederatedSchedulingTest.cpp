#include "FederatedSchedulingTest.h"
#include "FederatedScheduler.h"
#include "FileLock.h"
#include <iostream>


void FederatedSchedulingUnitTestManual() 
{
	std::string TasksStr = std::read_file("TaskSetTest20240216.json");
	//std::cout << TasksStr << std::endl;
	nlohmann::json TaskDoc = nlohmann::json::parse(TasksStr);
	STasks tasks{ TaskDoc.template get<STasks>() };

	FederatedScheduler fscheduler(10);

	for (int t = 0; t < tasks.tasks.size(); ++t) {
		fscheduler.add_task(tasks.tasks[t]);

		const STask& task_detail = fscheduler.get_tasks().back().task_detail();
		std::cout << "************************************ TASK DETAIL ***************************************" << std::endl;
		std::cout << "Task ID: " << task_detail.task_id << std::endl;
		std::cout << "Release Time: " << task_detail.release_time << std::endl;
		std::cout << "Period: " << task_detail.period << std::endl;
		std::cout << "Deadline: " << task_detail.deadline << std::endl;
		std::cout << "Priority: " << task_detail.priority << std::endl;
		std::cout << "Jobs: " << std::endl;
		for (auto iter = task_detail.jobs.begin(); iter != task_detail.jobs.end(); ++iter) {
			std::cout << "{ " << std::endl;
			std::cout << "\tJob ID: " << iter->second.job_id << std::endl;
			std::cout << "\tExecution Time: " << iter->second.execution_time << std::endl;
			std::cout << "\tNext Jobs: ";
			for (int j = 0; j < iter->second.next_jobs.size(); ++j) std::cout << iter->second.next_jobs[j] << " ";
			std::cout << std::endl;
			std::cout << "\tPrevious Jobs: ";
			for (int j = 0; j < iter->second.previous_jobs.size(); ++j) std::cout << iter->second.previous_jobs[j] << " ";
			std::cout << std::endl;
			std::cout << "}" << std::endl;
		}
		std::cout << "************************************ *********** ***************************************" << std::endl;
	}

	fscheduler.add_active_processors(std::vector<ActiveVirtualProcessor>{
		ActiveVirtualProcessor(-1, 1200, 0),
			ActiveVirtualProcessor(-1, 100, 0),
			ActiveVirtualProcessor(-1, 1600, 0)
	});
	bool schedulable = fscheduler.is_schedulable_on_active_processors(fscheduler.get_tasks()[0]);
	std::cout << "Task 0 schedulable on active processors: " << schedulable << std::endl;

	schedulable = fscheduler.is_schedulable_on_passive_processors(fscheduler.get_tasks()[1], std::vector<int>{0, 1});
	std::cout << "Task 1 schedulable on passive processors: " << schedulable << std::endl;

	schedulable = fscheduler.is_schedulable_on_mixed_processors(fscheduler.get_tasks()[1], std::vector<int>{2}, std::vector<int>{0, 1, 2});
	std::cout << "Task 1 schedulable on mixed processors: " << schedulable << std::endl;
}
