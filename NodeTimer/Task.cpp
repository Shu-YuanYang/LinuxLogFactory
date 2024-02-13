#include "Task.h"


Task::Task(int task_id, int release_time, int deadline, int period, int priority) : task{ task_id, release_time, 0, deadline, period, priority, std::map<int, SJob>() }, starting_job_id(-1), ending_job_id(-1)
{
	
}

void Task::set_job_map(const std::vector<SJob>& jobs) {
	this->task.jobs.clear();
	for (std::vector<SJob>::const_iterator iter(jobs.begin()); iter != jobs.end(); ++iter) {
		auto result = this->task.jobs.insert({ iter->job_id, *iter });
		result.first->second.previous_jobs.clear();		// recompute previous jobs instead of manual entry
		if (iter->next_jobs.empty()) this->ending_job_id = iter->job_id;
		//if (iter->previous_jobs.empty()) this->starting_job_id = iter->job_id;
	}

	//SJob& ending_job = this->task.jobs.at(this->ending_job_id);
	for (std::vector<SJob>::const_iterator iter(jobs.begin()); iter != jobs.end(); ++iter) {
		for (int i = 0; i < iter->next_jobs.size(); ++i) {
			this->task.jobs.at(iter->next_jobs[i]).previous_jobs.push_back(iter->job_id);
		}
	}
}

void Task::set_job_map(const std::map<int, SJob>& jobs) {
	this->task.jobs = jobs;
	for (std::map<int, SJob>::iterator iter(this->task.jobs.begin()); iter != this->task.jobs.end(); ++iter) {
		iter->second.previous_jobs.clear();		// recompute previous jobs instead of manual entry
		if (iter->second.next_jobs.empty()) this->ending_job_id = iter->second.job_id;
	}

	for (std::map<int, SJob>::iterator iter(this->task.jobs.begin()); iter != this->task.jobs.end(); ++iter) {
		for (int i = 0; i < iter->second.next_jobs.size(); ++i) {
			this->task.jobs.at(iter->second.next_jobs[i]).previous_jobs.push_back(iter->second.job_id);
		}
	}
}

const STask& Task::task_detail() const {
	return this->task;
}

std::vector<const SJob* > Task::get_longest_path(int& longest_execution_time) const {
	return this->get_longest_path(this->ending_job_id, longest_execution_time);
}

std::vector<const SJob* > Task::get_longest_path(int end_at_job_id, int& longest_execution_time) const {
	
	const SJob& current_job = this->task.jobs.at(end_at_job_id);
	const std::vector<int>& previous_jobs = current_job.previous_jobs;
	std::vector<const SJob* > longest_path;
	longest_execution_time = 0;

	for (int i = 0; i < previous_jobs.size(); ++i) {
		int path_execution_time;
		std::vector<const SJob* > path = this->get_longest_path(previous_jobs[i], path_execution_time);

		if (longest_execution_time < path_execution_time) {
			longest_path = path;
			longest_execution_time = path_execution_time;
		}
	}

	longest_path.push_back(&(current_job));
	longest_execution_time += current_job.execution_time;

	return longest_path;
}


int Task::get_total_execution_time() const {
	int total_execution_timer = 0;
	for (std::map<int, SJob>::const_iterator iter(this->task.jobs.begin()); iter != this->task.jobs.end(); ++iter) {
		total_execution_timer += iter->second.execution_time;
	}
	return total_execution_timer;
}