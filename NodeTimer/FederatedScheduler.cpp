#include "FederatedScheduler.h"
#include <cmath>
#include <algorithm>



ActiveVirtualProcessor::ActiveVirtualProcessor(int proc_id, int processor_budget, int task_id) : VirtualProcessor{ proc_id, processor_budget }, dedicated_task_id(task_id) {}


FederatedScheduler::FederatedScheduler(int processor_count) : __processor_count__(processor_count), __max_processor_id__(0), tasks{}, active_virtual_processor_refs{}, __active_virtual_processors__{}, passive_virtual_processor_refs{}, __passive_virtual_processors__{} {}

void FederatedScheduler::reset(int processor_count) {
	this->__processor_count__ = processor_count;
	this->__max_processor_id__ = 0;
	this->tasks.clear();
	this->active_virtual_processor_refs.clear();
	this->passive_virtual_processor_refs.clear();
	this->__active_virtual_processors__.clear();
	this->__passive_virtual_processors__.clear();
}


void FederatedScheduler::add_task(STask& task) {
	task.task_id = this->tasks.size();
	this->tasks.push_back(Task(task));
}

void FederatedScheduler::add_task(Task& task) {
	const STask& task_detail = task.task_detail();
	if (task_detail.task_id != tasks.size()) {
		Task aligned_task(this->tasks.size(), task_detail.release_time, task_detail.deadline, task_detail.period, task_detail.priority);
		aligned_task.set_job_map(task_detail.jobs);
		task = aligned_task;
	}
	this->tasks.push_back(task);
}

const std::vector<Task>& FederatedScheduler::get_tasks() const {
	return this->tasks;
}

int FederatedScheduler::get_processor_count() const {
	return this->__max_processor_id__;
}

int FederatedScheduler::compute_processor_demand(const Task& task) const {
	
	int longest_execution_time;
	std::vector<const SJob* > longest_path = task.get_longest_path(longest_execution_time);
	int total_execution_time = task.get_total_execution_time();
	int deadline = task.task_detail().deadline;

	int processor_demand = std::ceil( double(total_execution_time - longest_execution_time) / double(deadline - longest_execution_time) );
	if (processor_demand == 0 && 0 < longest_execution_time) processor_demand = 1; // Shu-Yuan Yang ammended in case there is only one path in the graph
	return processor_demand;
}



void FederatedScheduler::add_active_processors(const std::vector<ActiveVirtualProcessor>& virtual_processors) {
	
	std::vector<int> active_processor_refs;
	std::vector<int> passive_processor_refs;
	for (int i = 0; i < virtual_processors.size(); ++i) {
		this->__active_virtual_processors__.push_back(virtual_processors[i]);
		this->__active_virtual_processors__.back().processor_id = this->__max_processor_id__;
		active_processor_refs.push_back(this->__max_processor_id__);

		this->__passive_virtual_processors__.push_back(VirtualProcessor{ this->__max_processor_id__, 0 });
		passive_processor_refs.push_back(this->__max_processor_id__);
		++(this->__max_processor_id__);
	}
	this->active_virtual_processor_refs.insert({ virtual_processors.front().dedicated_task_id, active_processor_refs });
	if (this->passive_virtual_processor_refs.find(-1) == this->passive_virtual_processor_refs.end()) this->passive_virtual_processor_refs.insert({ -1, passive_processor_refs });
	else {
		std::vector<int>& existing_passive_procssor_refs = this->passive_virtual_processor_refs.at(-1);
		existing_passive_procssor_refs.insert(existing_passive_procssor_refs.end(), passive_processor_refs.begin(), passive_processor_refs.end());
	}
}

void FederatedScheduler::remove_last_active_processors(int size) {
	int task_id;
	this->__max_processor_id__ -= size;
	std::vector<ActiveVirtualProcessor>::iterator iter(this->__active_virtual_processors__.begin());
	std::vector<VirtualProcessor>::iterator piter(this->__passive_virtual_processors__.begin());
	iter += this->__max_processor_id__;
	piter += this->__max_processor_id__;
	while (iter != this->__active_virtual_processors__.end()) {
		int proc_id = iter->processor_id;
		task_id = iter->dedicated_task_id;

		// remove instantiated virtual processors:
		iter = this->__active_virtual_processors__.erase(iter);
		piter = this->__passive_virtual_processors__.erase(piter);

		// remove passive processor references:
		std::vector<int>& existing_passive_procssor_refs = this->passive_virtual_processor_refs.at(-1);
		for (std::vector<int>::iterator pr_iter(existing_passive_procssor_refs.begin()); pr_iter != existing_passive_procssor_refs.end(); ++pr_iter) {
			if (*pr_iter == proc_id) {
				existing_passive_procssor_refs.erase(pr_iter);
				break;
			}
		}
	}
	// remove active processor references:
	this->active_virtual_processor_refs.erase(this->active_virtual_processor_refs.find(task_id));
	
}


// Test based on Theorem 1, Page 485
bool FederatedScheduler::is_schedulable_on_active_processors(const Task& task) const {
	
	const STask& task_detail = task.task_detail();

	// 1. total execution time:
	int total_budget = 0;
	const std::vector<int>& dedicated_processor_ids = this->active_virtual_processor_refs.at(task_detail.task_id);
	for (int i = 0; i < dedicated_processor_ids.size(); ++i) total_budget += this->__active_virtual_processors__[dedicated_processor_ids[i]].budget;
	int total_execution_time = task.get_total_execution_time();

	if (total_budget != total_execution_time) return false;


	// 2. first processor deadline test:
	if (task_detail.deadline < this->__active_virtual_processors__[dedicated_processor_ids.front()].budget) return false;
	

	// 3. remaining processors deadline test:
	int longest_execution_time;
	task.get_longest_path(longest_execution_time);
	int gap = task_detail.deadline - longest_execution_time;
	for (int i = 1; i < dedicated_processor_ids.size(); ++i) {
		if (gap < this->__active_virtual_processors__[dedicated_processor_ids[i]].budget) return false;
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
	const ActiveVirtualProcessor& active_processor = this->__active_virtual_processors__[passive_virtual_processor_id];
	if (interval < active_processor.budget) return 0;
	
	const Task& dedicated_task = this->tasks[active_processor.dedicated_task_id];
	int alpha = supply_bound_alpha_function(active_processor, dedicated_task, interval);
	int gamma = supply_bound_gamma_function(active_processor, dedicated_task, interval);
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
		const ActiveVirtualProcessor& active_processor = this->__active_virtual_processors__[active_virtual_processor_ids[i]];
		budgets[i] = active_processor.budget;
		total_budget += active_processor.budget;
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


// Based on observation (17), Page 490
bool FederatedScheduler::is_useful_passive_processor(int passive_virtual_processor_id, const Task& task) const {
	const STask& task_detail = task.task_detail();
	int supply_bound = this->supply_bound_function(passive_virtual_processor_id, task_detail.deadline);
	int longest_execution_time;
	task.get_longest_path(longest_execution_time);
	return longest_execution_time < supply_bound;
}




// Based on the resource allocation algorithm, Page 491
void FederatedScheduler::schedule_task_set(bool& schedulable) {
	// 1. Sort tasks by D - L in ascending order:
	std::vector<Task* > __ordered_tasks(this->tasks.size());
	for (int i = 0; i < this->tasks.size(); ++i) __ordered_tasks[i] = &(this->tasks[i]);
	std::sort(__ordered_tasks.begin(), __ordered_tasks.end(), [](const Task* lhs, const Task* rhs) -> bool {
		int lhs_longest_execution_time, rhs_longest_execution_time;
		lhs->get_longest_path(lhs_longest_execution_time);
		rhs->get_longest_path(rhs_longest_execution_time);
		int lhs_gap = lhs->task_detail().deadline - lhs_longest_execution_time;
		int rhs_gap = rhs->task_detail().deadline - rhs_longest_execution_time;
		return lhs_gap < rhs_gap;
	});


	schedulable = false;
	for (int i = 0; i < __ordered_tasks.size(); ++i) {
		int processor_demand = this->compute_processor_demand(*(__ordered_tasks[i]));
		
		const STask& task_detail = __ordered_tasks[i]->task_detail();
		int longest_execution_time;
		__ordered_tasks[i]->get_longest_path(longest_execution_time);
		int total_execution_time = __ordered_tasks[i]->get_total_execution_time();


		// all jobs on active processors:
		if (processor_demand <= this->__processor_count__ - this->__active_virtual_processors__.size()) {
			std::vector<ActiveVirtualProcessor> active_processors(processor_demand, ActiveVirtualProcessor(-1, task_detail.deadline - longest_execution_time, task_detail.task_id));
			active_processors.front().budget = task_detail.deadline;
			if (1 < active_processors.size()) { 
				int execution_excess = total_execution_time - task_detail.deadline;
				int gap = task_detail.deadline - longest_execution_time;
				int remainder = execution_excess % gap;
				if (remainder != 0) active_processors.back().budget = remainder; //(execution_excess - int(std::floor(double(execution_excess) / double(gap))) * gap);
			}
			this->add_active_processors(active_processors);
		}
		// some jobs on active processors, and some on passive:
		else if (this->__active_virtual_processors__.size() < this->__processor_count__) {
			std::vector<ActiveVirtualProcessor> active_processors(this->__processor_count__ - this->__active_virtual_processors__.size(), ActiveVirtualProcessor(-1, task_detail.deadline - longest_execution_time, task_detail.task_id));
			active_processors.front().budget = task_detail.deadline;
			this->add_active_processors(active_processors);

			std::vector<int> active_processor_refs = this->active_virtual_processor_refs.at(task_detail.task_id);
			//std::vector<int> active_processor_ids(active_processor_refs.size()); 
			//for (int j = 0; j < active_processor_refs.size(); ++j) active_processor_ids[j] = active_processor_refs[i]->processor_id;
			std::vector<int> passive_processor_ids;
			while (!this->is_schedulable_on_mixed_processors(*(__ordered_tasks[i]), active_processor_refs, passive_processor_ids)) {
				std::vector<int>& unused_passive_processor_refs = this->passive_virtual_processor_refs.at(-1);
				std::vector<int>::iterator iter = unused_passive_processor_refs.begin();
				while (iter != unused_passive_processor_refs.end() && !this->is_useful_passive_processor(*iter, *(__ordered_tasks[i]))) ++iter;
				if (iter == unused_passive_processor_refs.end()) return;

				passive_processor_ids.push_back(*iter);
				//if (this->passive_virtual_processor_refs.find(task_detail.task_id) == this->passive_virtual_processor_refs.end()) this->passive_virtual_processor_refs.insert({ task_detail.task_id, std::vector<int>{ *iter } });
				//else this->passive_virtual_processor_refs.at(task_detail.task_id).push_back(*iter);
				unused_passive_processor_refs.erase(iter);
			}
			this->passive_virtual_processor_refs.insert({ task_detail.task_id, passive_processor_ids });
		}
		// all jobs on passive processors:
		else {
			std::vector<int> passive_processor_ids;
			while (!this->is_schedulable_on_passive_processors(*(__ordered_tasks[i]), passive_processor_ids)) {
				std::vector<int>& unused_passive_processor_refs = this->passive_virtual_processor_refs.at(-1);
				std::vector<int>::iterator iter = unused_passive_processor_refs.begin();
				while (iter != unused_passive_processor_refs.end() && !this->is_useful_passive_processor(*iter, *(__ordered_tasks[i]))) ++iter;
				if (iter == unused_passive_processor_refs.end()) return;

				passive_processor_ids.push_back(*iter);
				//if (this->passive_virtual_processor_refs.find(task_detail.task_id) == this->passive_virtual_processor_refs.end()) this->passive_virtual_processor_refs.insert({ task_detail.task_id, std::vector<VirtualProcessor* >{ *iter } });
				//else this->passive_virtual_processor_refs.at(task_detail.task_id).push_back(*iter);
				unused_passive_processor_refs.erase(iter);
			}
			this->passive_virtual_processor_refs.insert({ task_detail.task_id, passive_processor_ids });
		}
	}

	schedulable = true;
}


void FederatedScheduler::schedule_task_set_uncommitted(bool& schedulable) {
	// 1. Sort tasks by D - L in ascending order:
	std::vector<Task* > __ordered_tasks(this->tasks.size());
	for (int i = 0; i < this->tasks.size(); ++i) __ordered_tasks[i] = &(this->tasks[i]);
	std::sort(__ordered_tasks.begin(), __ordered_tasks.end(), [](const Task* lhs, const Task* rhs) -> bool {
		int lhs_longest_execution_time, rhs_longest_execution_time;
		lhs->get_longest_path(lhs_longest_execution_time);
		rhs->get_longest_path(rhs_longest_execution_time);
		int lhs_gap = lhs->task_detail().deadline - lhs_longest_execution_time;
		int rhs_gap = rhs->task_detail().deadline - rhs_longest_execution_time;
		return lhs_gap < rhs_gap;
	});


	schedulable = false;
	for (int i = 0; i < __ordered_tasks.size(); ++i) {
		int processor_demand = this->compute_processor_demand(*(__ordered_tasks[i]));

		const STask& task_detail = __ordered_tasks[i]->task_detail();
		int longest_execution_time;
		__ordered_tasks[i]->get_longest_path(longest_execution_time);
		int total_execution_time = __ordered_tasks[i]->get_total_execution_time();


		bool is_uncommitted = false;

		// all jobs on active processors:
		if (processor_demand <= this->__processor_count__ - this->__active_virtual_processors__.size()) {
			std::vector<ActiveVirtualProcessor> active_processors(processor_demand, ActiveVirtualProcessor(-1, task_detail.deadline - longest_execution_time, task_detail.task_id));
			active_processors.front().budget = task_detail.deadline;
			if (1 < active_processors.size()) {
				int execution_excess = total_execution_time - task_detail.deadline;
				int gap = task_detail.deadline - longest_execution_time;
				int remainder = execution_excess % gap;
				if (remainder != 0) active_processors.back().budget = remainder; //(execution_excess - int(std::floor(double(execution_excess) / double(gap))) * gap);
			}
			this->add_active_processors(active_processors);
		}
		// some jobs on active processors, and some on passive:
		else if (this->__active_virtual_processors__.size() < this->__processor_count__) {
			std::vector<ActiveVirtualProcessor> active_processors(this->__processor_count__ - this->__active_virtual_processors__.size(), ActiveVirtualProcessor(-1, task_detail.deadline - longest_execution_time, task_detail.task_id));
			active_processors.front().budget = task_detail.deadline;
			this->add_active_processors(active_processors);

			std::vector<int> active_processor_refs = this->active_virtual_processor_refs.at(task_detail.task_id);
			//std::vector<int> active_processor_ids(active_processor_refs.size()); 
			//for (int j = 0; j < active_processor_refs.size(); ++j) active_processor_ids[j] = active_processor_refs[i]->processor_id;
			std::vector<int> passive_processor_ids;
			std::vector<int> passive_processor_id_indices;
			int index = 0;
			std::vector<int>& unused_passive_processor_refs = this->passive_virtual_processor_refs.at(-1);
			while (!this->is_schedulable_on_mixed_processors(*(__ordered_tasks[i]), active_processor_refs, passive_processor_ids)) {
				while (index < unused_passive_processor_refs.size() && !this->is_useful_passive_processor(unused_passive_processor_refs[index], *(__ordered_tasks[i]))) ++index;
				if (index == unused_passive_processor_refs.size()) {
					is_uncommitted = true;
					break;
				};

				passive_processor_ids.push_back(unused_passive_processor_refs[index]);
				passive_processor_id_indices.push_back(index);
				++index;
			}

			if (is_uncommitted) {
				this->uncommitted_refs.insert(task_detail.task_id);
				this->remove_last_active_processors(active_processors.size());
				continue;
			}

			// adjust passive processor assignments:
			std::vector<int>::iterator iter(unused_passive_processor_refs.begin());
			int prev_index = 0;
			for (int id_ind = 0 ; id_ind < passive_processor_id_indices.size() && iter != unused_passive_processor_refs.end(); ++id_ind) {
				int diff = passive_processor_id_indices[id_ind] - prev_index;
				iter += diff;
				iter = unused_passive_processor_refs.erase(iter);
				prev_index = passive_processor_id_indices[id_ind] + 1;
			}
			this->passive_virtual_processor_refs.insert({ task_detail.task_id, passive_processor_ids });
		}
		// all jobs on passive processors:
		else {
			std::vector<int> passive_processor_ids;
			std::vector<int> passive_processor_id_indices;
			int index = 0;
			std::vector<int>& unused_passive_processor_refs = this->passive_virtual_processor_refs.at(-1);
			while (!this->is_schedulable_on_passive_processors(*(__ordered_tasks[i]), passive_processor_ids)) {
				while (index < unused_passive_processor_refs.size() && !this->is_useful_passive_processor(unused_passive_processor_refs[index], *(__ordered_tasks[i]))) ++index;
				if (index == unused_passive_processor_refs.size()) {
					is_uncommitted = true;
					break;
				};

				passive_processor_ids.push_back(unused_passive_processor_refs[index]);
				passive_processor_id_indices.push_back(index);
				++index;
			}

			if (is_uncommitted) {
				this->uncommitted_refs.insert(task_detail.task_id);
				continue;
			}

			// adjust passive processor assignments:
			std::vector<int>::iterator iter(unused_passive_processor_refs.begin());
			int prev_index = 0;
			for (int id_ind = 0; id_ind < passive_processor_id_indices.size() && iter != unused_passive_processor_refs.end(); ++id_ind) {
				int diff = passive_processor_id_indices[id_ind] - prev_index;
				iter += diff;
				iter = unused_passive_processor_refs.erase(iter);
				prev_index = passive_processor_id_indices[id_ind] + 1;
			}
			this->passive_virtual_processor_refs.insert({ task_detail.task_id, passive_processor_ids });
		}
	}

	if (uncommitted_refs.empty()) schedulable = true;
}




std::vector<ProcessorAssignment> FederatedScheduler::get_processor_assignments() {
	
	std::vector<ProcessorAssignment> assignments(this->tasks.size());
	for (int i = 0; i < this->tasks.size(); ++i) {
		assignments[i].task_ref = &(this->tasks[i]);

		int task_id = this->tasks[i].task_detail().task_id;

		bool are_active_processors_assigned = (this->active_virtual_processor_refs.find(task_id) != this->active_virtual_processor_refs.end());
		bool are_passive_processors_assigned = (this->passive_virtual_processor_refs.find(task_id) != this->passive_virtual_processor_refs.end());
		if (are_active_processors_assigned) {
			const std::vector<int>& processor_ids = this->active_virtual_processor_refs.at(task_id);
			for (int j = 0; j < processor_ids.size(); ++j) assignments[i].active_virtual_processor_refs.push_back(&(this->__active_virtual_processors__[processor_ids[j]]));
		}

		if (are_passive_processors_assigned) {
			const std::vector<int>& processor_ids = this->passive_virtual_processor_refs.at(task_id);
			for (int j = 0; j < processor_ids.size(); ++j) assignments[i].passive_virtual_processor_refs.push_back(&(this->__passive_virtual_processors__[processor_ids[j]]));
		}

		assignments[i].is_uncommitted = !(are_active_processors_assigned || are_passive_processors_assigned);
	}

	return assignments;

}



void FederatedScheduler::renew_period(int task_id, int current_time_step) {
	Task& task_ref = this->tasks[task_id];
	//const STask& task_detail = task_ref.task_detail();
	//if (current_time_step - task_detail.release_time == task_detail.period) {
	task_ref.renew_period(current_time_step);
	std::map<int, std::vector<int> >::iterator virtual_processor_refs_iter(this->active_virtual_processor_refs.find(task_id));
	if (virtual_processor_refs_iter == this->active_virtual_processor_refs.end()) return;

	const std::vector<int>& active_processor_ids = virtual_processor_refs_iter->second;
	for (std::vector<int>::const_iterator iter(active_processor_ids.begin()); iter != active_processor_ids.end(); ++iter) {
		this->__active_virtual_processors__[*iter].__used_budget__ = 0;
	}
	//}
}

void FederatedScheduler::update_job_progress(int task_id, int job_id, int processor_id, bool is_active_processor, int time_units) {
	this->tasks[task_id].update_job_progress(job_id, time_units);
	if (is_active_processor) this->__active_virtual_processors__[processor_id].__used_budget__ += time_units;
	//else this->__passive_virtual_processors__[processor_id].__used_budget__;
}