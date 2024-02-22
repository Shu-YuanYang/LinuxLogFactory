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


struct TaskJobIndex {
public:
	int task_id;
	int job_id;
	int laxity;
};

void RunScheduleSimulation(FederatedScheduler& scheduler) {

	std::vector<ProcessorAssignment> assignments(scheduler.get_processor_assignments());
	
	int time_unit = 100;
	int hyper_period = 1;
	for (int i = 0; i < assignments.size(); ++i) hyper_period = lcm(assignments[i].task_ref->task_detail().period, hyper_period);

	// Run timeline:
	for (int step = 0; step < hyper_period; step += time_unit) {

		std::vector<TaskJobIndex> active_processor_scheduled_jobs(scheduler.get_processor_count(), TaskJobIndex{ -1, -1, INT32_MAX });
		std::vector<TaskJobIndex> passive_processor_scheduled_jobs(scheduler.get_processor_count(), TaskJobIndex{ -1, -1, INT32_MAX });

		// Period update and active processor resource allocation:
		for (int i = 0; i < assignments.size(); ++i) {
			Task* task_ref = assignments[i].task_ref;
			const STask& task_detail = task_ref->task_detail();

			// check for deadline misses
			std::map<int, int> eligible_deadlines( task_ref->get_eligible_absolute_deadlines() );
			for (std::map<int, int>::const_iterator iter(eligible_deadlines.begin()); iter != eligible_deadlines.end(); ++iter) {
				const SJob& job_ref = task_detail.jobs.at(iter->first);
				if (step == iter->second && job_ref.progress < job_ref.execution_time) {
					std::cout << "Task ID: " << std::to_string(task_detail.task_id) << ", Job ID: " << std::to_string(iter->first) << " missed the deadline. Absolute deadline: " << std::to_string(iter->second) << ", progress: " << std::to_string(job_ref.progress);
					return;
				}
			}
			
			// check for period renewal
			if (step - task_detail.release_time == task_detail.period) scheduler.renew_period(task_detail.task_id, step);
			
			// determine eligible jobs for active processors (LLF): 
			eligible_deadlines = task_ref->get_eligible_absolute_deadlines();
			for (std::map<int, int>::const_iterator iter(eligible_deadlines.begin()); iter != eligible_deadlines.end(); ++iter) {
				// ensure job is not scheduled to run on a processor already.
				bool job_scheduled = false;
				for (int p = 0; p < active_processor_scheduled_jobs.size(); ++p) {
					if (active_processor_scheduled_jobs[p].task_id == task_detail.task_id && active_processor_scheduled_jobs[p].job_id == iter->first) {
						job_scheduled = true;
						break;
					}
				}
				if (job_scheduled) continue;


				const SJob& job_ref = task_detail.jobs.at(iter->first);
				int laxity = task_ref->get_job_laxity(iter->first, step);
				for (int p = 0; p < assignments[i].active_virtual_processor_refs.size(); ++p) {
					int processor_id = assignments[i].active_virtual_processor_refs[p]->processor_id;
					if (assignments[i].active_virtual_processor_refs[p]->__used_budget__ == assignments[i].active_virtual_processor_refs[p]->budget) continue; // active processor budget exhausted
					if (laxity < active_processor_scheduled_jobs[processor_id].laxity) {
						active_processor_scheduled_jobs[processor_id].task_id = task_detail.task_id;
						active_processor_scheduled_jobs[processor_id].job_id = job_ref.job_id;
						active_processor_scheduled_jobs[processor_id].laxity = laxity;
						break; // move on to the next job if schedules.
					}
				}
			}
		}

		// Passive processor resource allocation:
		for (int i = 0; i < assignments.size(); ++i) {

			Task* task_ref = assignments[i].task_ref;
			const STask& task_detail = task_ref->task_detail();
			// determine eligible jobs for passive processors (LLF):
			
			std::map<int, int> eligible_deadlines = task_ref->get_eligible_absolute_deadlines();
			for (std::map<int, int>::const_iterator iter(eligible_deadlines.begin()); iter != eligible_deadlines.end(); ++iter) {
				// ensure job is not scheduled to run on a processor already.
				bool job_scheduled = false;
				for (int p = 0; p < active_processor_scheduled_jobs.size(); ++p) {
					if (active_processor_scheduled_jobs[p].task_id == task_detail.task_id && active_processor_scheduled_jobs[p].job_id == iter->first) {
						job_scheduled = true;
						break;
					}
				}
				if (job_scheduled) continue;
				for (int p = 0; p < passive_processor_scheduled_jobs.size(); ++p) {
					if (passive_processor_scheduled_jobs[p].task_id == task_detail.task_id && passive_processor_scheduled_jobs[p].job_id == iter->first) {
						job_scheduled = true;
						break;
					}
				}
				if (job_scheduled) continue;

				// passive processors determine eligible job based on laxity:
				const SJob& job_ref = task_detail.jobs.at(iter->first);
				int laxity = task_ref->get_job_laxity(iter->first, step);
				for (int p = 0; p < assignments[i].passive_virtual_processor_refs.size(); ++p) {
					int processor_id = assignments[i].passive_virtual_processor_refs[p]->processor_id;
					if (active_processor_scheduled_jobs[processor_id].task_id != -1) continue; // active processor is not idle, do not use passive processor
					if (laxity < passive_processor_scheduled_jobs[processor_id].laxity) {
						passive_processor_scheduled_jobs[processor_id].task_id = task_detail.task_id;
						passive_processor_scheduled_jobs[processor_id].job_id = job_ref.job_id;
						passive_processor_scheduled_jobs[processor_id].laxity = laxity;
						break; // move on to the next job if schedules.
					}
				}
			}
		}
		

		// for each processor, update job progress
		std::cout << std::to_string(step) << " -> ";
		for (int p = 0; p < active_processor_scheduled_jobs.size(); ++p) {
			int task_id_active = active_processor_scheduled_jobs[p].task_id;
			int job_id_active = active_processor_scheduled_jobs[p].job_id;

			int task_id_passive = passive_processor_scheduled_jobs[p].task_id;
			int job_id_passive = passive_processor_scheduled_jobs[p].job_id;
			
			
			if (task_id_active != -1) 
			{
				scheduler.update_job_progress(task_id_active, job_id_active, p, true, time_unit);
				std::cout << "a" << std::to_string(p) << ": " << "{t:" << task_id_active << ",j:" << job_id_active << "} ";
			}
			else if (task_id_passive != -1)
			{
				scheduler.update_job_progress(task_id_passive, job_id_passive, p, false, time_unit);
				std::cout << "p" << std::to_string(p) << ": " "{t:" << task_id_passive << ",j:" << job_id_passive << "} ";
			}
			else {
				std::cout << "n" << std::to_string(p) << ": {t:n,j:n} ";
			}
		}
		std::cout << std::endl;

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

	RunScheduleSimulation(fscheduler);
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