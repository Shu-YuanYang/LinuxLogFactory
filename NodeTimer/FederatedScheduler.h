#pragma once


#include "Task.h"
#include <set>


struct VirtualProcessor {
public:
	int processor_id;
	int budget;
	int __used_budget__;
};

struct ActiveVirtualProcessor : public VirtualProcessor {
public:
	int dedicated_task_id;

	ActiveVirtualProcessor(int processor_id, int processor_budget, int task_id);
};


struct ProcessorAssignment {
public:
	Task* task_ref = nullptr;
	std::vector<const ActiveVirtualProcessor* > active_virtual_processor_refs;
	std::vector<const VirtualProcessor* > passive_virtual_processor_refs;
	bool is_uncommitted;
};





class FederatedScheduler
{

public:

	FederatedScheduler(int proceessor_count);

	void add_task(STask& task);

	void add_task(Task& task);

	const std::vector<Task>& get_tasks() const;

	std::vector<ProcessorAssignment> get_processor_assignments();

	int get_processor_count() const;

	int compute_processor_demand(const Task& task) const;

	bool is_schedulable_on_active_processors(const Task& task) const;

	bool is_schedulable_on_passive_processors(const Task& task, std::vector<int> passive_virtual_processor_ids) const;

	bool is_schedulable_on_mixed_processors(const Task& task, std::vector<int> active_virtual_processor_ids, std::vector<int> passive_virtual_processor_ids) const;

	void add_active_processors(const std::vector<ActiveVirtualProcessor>& virtual_processors);

	int supply_bound_function(int passive_virtual_processor_id, int interval) const;

	bool is_useful_passive_processor(int passive_virtual_processor_id, const Task& task) const;

	void schedule_task_set(bool& schedulable);

	void schedule_task_set_uncommitted(bool& schedulable);

	void reset(int processor_count);

	void renew_period(int task_id, int current_time_step);

	void update_job_progress(int task_id, int job_id, int processor_id, bool is_active_processor, int time_units);

private:

	int __processor_count__;
	int __max_processor_id__ = 0;
	std::vector<Task> tasks;
	std::map<int, std::vector<int> > active_virtual_processor_refs;
	std::map<int, std::vector<int> > passive_virtual_processor_refs;
	std::set<int> uncommitted_refs;
	std::vector<ActiveVirtualProcessor> __active_virtual_processors__;
	std::vector<VirtualProcessor> __passive_virtual_processors__;
	
	void remove_last_active_processors(int size);

	/*
	int supply_bound_alpha_function(const ActiveVirtualProcessor& active_virtual_processor, const Task& task, int interval) const;
	int supply_bound_beta_function(const ActiveVirtualProcessor& active_virtual_processor, const Task& task, int interval) const;
	int supply_bound_gamma_function(const ActiveVirtualProcessor& active_virtual_processor, const Task& task, int interval) const;
	*/
};

