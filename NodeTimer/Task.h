#pragma once

#include <vector>
#include <map>


struct SJob {
	int job_id;
	int release_time;	// at millisecond point
	int execution_time; // in milliseconds
	int deadline;		// relative, in milliseconds
	std::vector<int> next_jobs;
	std::vector<int> previous_jobs;
};

struct STask {
public:
	int task_id;
	int release_time;		// at millisecond point
	int execution_time; // in milliseconds
	int deadline;		// relative, in milliseconds
	int period;			// in milliseconds
	int priority;		// smaller numbers have higher priorities

	std::map<int, SJob> jobs;
};


class Task
{
public:

	Task(int task_id, int release_time, int deadline, int period, int priority);

	void set_job_map(const std::vector<SJob>& jobs);

	void set_job_map(const std::map<int, SJob>& jobs);

	const STask& task_detail() const;


	// Compute:
	std::vector<const SJob* > get_longest_path(int& longest_execution_time) const;

	int get_total_execution_time() const;


private:
	STask task;
	int starting_job_id;
	int ending_job_id;


	std::vector<const SJob* > get_longest_path(int start_at_job_id, int& longest_execution_time) const;
};

