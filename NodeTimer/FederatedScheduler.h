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

	FederatedScheduler();

	void add_task(Task& task);

	int compute_processor_demand(const Task& task) const;

	bool is_schedulable_on_active_processors(const Task& task) const;

	bool is_schedulable_on_passive_processors(const Task& task, std::vector<int> passive_virtual_processor_ids) const;

	bool is_schedulable_on_mixed_processors(const Task& task, std::vector<int> active_virtual_processor_ids, std::vector<int> passive_virtual_processor_ids) const;

	void add_active_processors(const std::vector<ActiveVirtualProcessor>& virtual_processors);

	int supply_bound_function(int passive_virtual_processor_id, int interval) const;

private:

	int __max_processor_id__ = 0;
	std::vector<Task> tasks;
	std::map<int, std::vector<ActiveVirtualProcessor> > active_virtual_processors;
	std::vector<ActiveVirtualProcessor* > __active_virtual_processor_refs__;
	std::vector<VirtualProcessor> passive_virtual_processors;
	

	/*
	int supply_bound_alpha_function(const ActiveVirtualProcessor& active_virtual_processor, const Task& task, int interval) const;
	int supply_bound_beta_function(const ActiveVirtualProcessor& active_virtual_processor, const Task& task, int interval) const;
	int supply_bound_gamma_function(const ActiveVirtualProcessor& active_virtual_processor, const Task& task, int interval) const;
	*/
};

