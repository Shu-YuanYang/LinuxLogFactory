#include "FederatedScheduler.h"
#include <cmath>



ActiveVirtualProcessor::ActiveVirtualProcessor(int proc_id, int processor_budget, int task_id) : VirtualProcessor{ proc_id, processor_budget }, dedicated_task_id(task_id) {}


FederatedScheduler::FederatedScheduler() : __max_processor_id__(0), tasks{}, active_virtual_processors {}, __active_virtual_processor_refs__{}, passive_virtual_processors{} {}


void FederatedScheduler::add_task(Task& task) {
	const STask& task_detail = task.task_detail();
	if (task_detail.task_id != tasks.size()) {
		Task aligned_task(this->tasks.size(), task_detail.release_time, task_detail.deadline, task_detail.period, task_detail.priority);
		aligned_task.set_job_map(task_detail.jobs);
		task = aligned_task;
	}
	this->tasks.push_back(task);
}

int FederatedScheduler::compute_processor_demand(const Task& task) const {
	
	int longest_execution_time;
	std::vector<const SJob* > longest_path = task.get_longest_path(longest_execution_time);
	int total_execution_time = task.get_total_execution_time();
	int deadline = task.task_detail().deadline;

	int processor_demand = std::ceil( double(total_execution_time - longest_execution_time) / double(deadline - longest_execution_time) );
	return processor_demand;
}



void FederatedScheduler::add_active_processors(const std::vector<ActiveVirtualProcessor>& virtual_processors) {
	auto result = this->active_virtual_processors.insert({ virtual_processors.front().dedicated_task_id, virtual_processors });
	std::vector<ActiveVirtualProcessor>& processor_refs = result.first->second;
	for (std::vector<ActiveVirtualProcessor>::iterator iter(processor_refs.begin()); iter != processor_refs.end(); ++iter) {
		iter->processor_id = this->__max_processor_id__;
		this->__active_virtual_processor_refs__.push_back(&(*iter));
		this->passive_virtual_processors.push_back(VirtualProcessor{ this->__max_processor_id__, 0 });
		++(this->__max_processor_id__);
	}
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





// Function based on Lemma 3, Page 486
int supply_bound_alpha_function(const ActiveVirtualProcessor& active_virtual_processor, const Task& task, int interval) {
	int period = task.task_detail().period;
	return int( std::floor(double(interval - active_virtual_processor.budget) / double(period)) * (period - active_virtual_processor.budget) );
}

int supply_bound_beta_function(const ActiveVirtualProcessor& active_virtual_processor, const Task& task, int interval) {
	int period = task.task_detail().period;
	int beta = (interval - active_virtual_processor.budget) % period;
	if (beta < 0) beta += period;
	return beta;
}

int supply_bound_gamma_function(const ActiveVirtualProcessor& active_virtual_processor, const Task& task, int interval) {
	const STask& task_detail = task.task_detail();
	int beta = supply_bound_beta_function(active_virtual_processor, task, interval);
	int T_minus_D = task_detail.period - task_detail.deadline;
	int T_minus_D_plus_Budget = T_minus_D + active_virtual_processor.budget;

	if (beta <= T_minus_D) return beta;		// beta(interval) <= T - D
	if (beta <= T_minus_D_plus_Budget) return T_minus_D;	// T - D < beta(interval) <= T - D + budget
	return beta - active_virtual_processor.budget;
}

int FederatedScheduler::supply_bound_function(int passive_virtual_processor_id, int interval) const {
	/*
	std::map<int, int> supply_bound_map;
	const std::vector<ActiveVirtualProcessor>& active_processors = this->active_virtual_processors.at(task.task_detail().task_id);
	for (std::vector<ActiveVirtualProcessor>::const_iterator iter(active_processors.begin()); iter != active_processors.end(); ++iter) {
		if (interval < iter->budget) supply_bound_map.insert(iter->processor_id, 0);
		else {
			int alpha = supply_bound_alpha_function(*iter, task, interval);
			int gamma = supply_bound_gamma_function(*iter, task, interval);
			supply_bound_map.insert(iter->processor_id, alpha + gamma); 
		}
	}
	*/
	const ActiveVirtualProcessor* active_processor = this->__active_virtual_processor_refs__[passive_virtual_processor_id];
	if (interval < active_processor->budget) return 0;
	
	const Task& dedicated_task = this->tasks[active_processor->dedicated_task_id];
	int alpha = supply_bound_alpha_function(*active_processor, dedicated_task, interval);
	int gamma = supply_bound_gamma_function(*active_processor, dedicated_task, interval);
	return alpha + gamma;
}



// test based on Theorem 2, Page 487
bool FederatedScheduler::is_schedulable_on_passive_processors(const Task& task, std::vector<int> passive_virtual_processor_ids) const {
	const STask& task_detail = task.task_detail();
	int longest_execution_time;
	task.get_longest_path(longest_execution_time);
	int demand = longest_execution_time;
	for (int i = 0; i < passive_virtual_processor_ids.size(); ++i)
		demand += std::max(this->supply_bound_function(passive_virtual_processor_ids[i], task_detail.deadline) - longest_execution_time, 0);

	int total_execution_time = task.get_total_execution_time();
	return total_execution_time <= demand;
}



// test based on Theorem 3, Page 488
bool FederatedScheduler::is_schedulable_on_mixed_processors(const Task& task, std::vector<int> active_virtual_processor_ids, std::vector<int> passive_virtual_processor_ids) const {
	int total_budget = 0;
	std::vector<int> budgets(active_virtual_processor_ids.size());
	for (int i = 0; i < active_virtual_processor_ids.size(); ++i) {
		const ActiveVirtualProcessor* active_processor_ref = this->__active_virtual_processor_refs__[active_virtual_processor_ids[i]];
		budgets[i] = active_processor_ref->budget;
		total_budget += active_processor_ref->budget;
	}

	const STask& task_detail = task.task_detail();
	int longest_execution_time;
	task.get_longest_path(longest_execution_time);
	int demand = 0;
	for (int i = 0; i < passive_virtual_processor_ids.size(); ++i)
		demand += std::max(this->supply_bound_function(passive_virtual_processor_ids[i], task_detail.deadline) - longest_execution_time, 0);
	
	int total_execution_time = task.get_total_execution_time();

	// 1. if execution time exceeds supply bounds, not schedulable.
	if (total_budget + demand < total_execution_time) return false;

	// 2. budget vs deadline test.
	if (task_detail.deadline < budgets[0]) return false;

	int gap = task_detail.deadline - longest_execution_time;
	for (int i = 1; i < budgets.size(); ++i) 
		if (gap < budgets[i]) return false;

	return true;
}