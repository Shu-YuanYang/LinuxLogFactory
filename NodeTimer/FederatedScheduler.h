#pragma once


#include "Task.h"


struct VirtualProcessor {
public:
	int processor_id;
	int budget;
};

struct ActiveVirtualProcessor : public VirtualProcessor {
public:
	int dedicated_task_id;

	ActiveVirtualProcessor(int processor_id, int processor_budget, int task_id);
};


class FederatedScheduler
{

public:

	FederatedScheduler(int proceessor_count);

	void add_task(STask& task);

	void add_task(Task& task);

	const std::vector<Task>& get_tasks() const;

	int compute_processor_demand(const Task& task) const;

	bool is_schedulable_on_active_processors(const Task& task) const;

	bool is_schedulable_on_passive_processors(const Task& task, std::vector<int> passive_virtual_processor_ids) const;

	bool is_schedulable_on_mixed_processors(const Task& task, std::vector<int> active_virtual_processor_ids, std::vector<int> passive_virtual_processor_ids) const;

	void add_active_processors(const std::vector<ActiveVirtualProcessor>& virtual_processors);

	int supply_bound_function(int passive_virtual_processor_id, int interval) const;

	bool is_useful_passive_processor(int passive_virtual_processor_id, const Task& task) const;

	void schedule_task_set(bool& schedulable);

	void reset(int processor_count);

private:

	int __processor_count__;
	int __max_processor_id__ = 0;
	std::vector<Task> tasks;
	std::map<int, std::vector<int> > active_virtual_processor_refs;
	std::map<int, std::vector<int> > passive_virtual_processor_refs;
	std::vector<ActiveVirtualProcessor> __active_virtual_processors__;
	std::vector<VirtualProcessor> __passive_virtual_processors__;
	

	/*
	int supply_bound_alpha_function(const ActiveVirtualProcessor& active_virtual_processor, const Task& task, int interval) const;
	int supply_bound_beta_function(const ActiveVirtualProcessor& active_virtual_processor, const Task& task, int interval) const;
	int supply_bound_gamma_function(const ActiveVirtualProcessor& active_virtual_processor, const Task& task, int interval) const;
	*/
};

