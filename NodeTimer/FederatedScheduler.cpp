#include "FederatedScheduler.h"
#include <cmath>



ActiveVirtualProcessor::ActiveVirtualProcessor(int processor_budget, int task_id) : VirtualProcessor{ processor_budget }, dedicated_task_id(task_id) {}


int FederatedScheduler::compute_processor_demand(const Task& task) const {
	
	int longest_execution_time;
	std::vector<const SJob* > longest_path = task.get_longest_path(longest_execution_time);
	int total_execution_time = task.get_total_execution_time();
	int deadline = task.task_detail().deadline;

	int processor_demand = std::ceil( double(total_execution_time - longest_execution_time) / double(deadline - longest_execution_time) );
	return processor_demand;
}



void FederatedScheduler::add_active_processors(const std::vector<ActiveVirtualProcessor>& virtual_processors) {
	this->active_virtual_processors.insert({ virtual_processors.front().dedicated_task_id, virtual_processors });
}


// Test based on Theorem 1, Page 485
bool FederatedScheduler::is_schedulable_on_active_processors(const Task& task) const {
	
	const STask& task_detail = task.task_detail();

	// 1. total execution time:
	int total_budget = 0;
	const std::vector<ActiveVirtualProcessor>& dedicated_processors = this->active_virtual_processors.at(task_detail.task_id);
	for (int i = 0; i < dedicated_processors.size(); ++i) total_budget += dedicated_processors[i].budget;
	int total_execution_time = task.get_total_execution_time();

	if (total_budget != total_execution_time) return false;


	// 2. first processor deadline test:
	if (task_detail.deadline < dedicated_processors.front().budget) return false;
	

	// 3. remaining processors deadline test:
	int longest_execution_time;
	task.get_longest_path(longest_execution_time);
	int gap = task_detail.deadline - longest_execution_time;
	for (int i = 1; i < dedicated_processors.size(); ++i) {
		if (gap < dedicated_processors[i].budget) return false;
	}

	return true;
}
