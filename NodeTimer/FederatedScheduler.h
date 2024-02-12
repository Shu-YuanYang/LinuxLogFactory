#pragma once


#include "Task.h"


struct VirtualProcessor {
public:
	int budget;
};

struct ActiveVirtualProcessor : public VirtualProcessor {
public:
	int dedicated_task_id;

	ActiveVirtualProcessor(int processor_budget, int task_id);
};


class FederatedScheduler
{

public:

	int compute_processor_demand(const Task& task) const;

	bool is_schedulable_on_active_processors(const Task& task) const;

	void add_active_processors(const std::vector<ActiveVirtualProcessor>& virtual_processors);


private:

	std::map<int, std::vector<ActiveVirtualProcessor> > active_virtual_processors;
	std::vector<VirtualProcessor> passive_virtual_processors;

};

