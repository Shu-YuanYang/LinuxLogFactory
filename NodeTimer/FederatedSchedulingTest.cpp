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
			std::cout << "\tDeadline: " << iter->second.deadline << std::endl;
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








// Least common multiple algorithm copied from: https://www.geeksforgeeks.org/program-to-find-lcm-of-two-numbers/
// Recursive function to return gcd of a and b  
long long gcd(long long int a, long long int b)
{
	if (b == 0)
		return a;
	return gcd(b, a % b);
}

// Function to return LCM of two numbers  
long long lcm(int a, int b)
{
	return (a / gcd(a, b)) * b;
}

void RunScheduleSimulation(const FederatedScheduler& scheduler) {

	std::vector<ProcessorAssignment> assignments(scheduler.get_processor_assignments());

	int time_unit = 100;
	int hyper_period = 1;
	for (int i = 0; i < assignments.size(); ++i) hyper_period = lcm(assignments[i].task_ref->task_detail().period, hyper_period);

	for (int step = 0; step < hyper_period; hyper_period += 100) {
		
	}

}



void FederatedSchedulingTestWithFileInput(const char* filename, int processor_count) {
	std::string TasksStr = std::read_file(filename);
	nlohmann::json TaskDoc = nlohmann::json::parse(TasksStr);
	STasks tasks{ TaskDoc.template get<STasks>() };

	FederatedScheduler fscheduler(processor_count);
	for (int t = 0; t < tasks.tasks.size(); ++t) fscheduler.add_task(tasks.tasks[t]);
	bool schedulable;
	fscheduler.schedule_task_set(schedulable);
	std::cout << "Task set schedulability: " << schedulable << std::endl;
	/*
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
	*/

	if (!schedulable) return;


}