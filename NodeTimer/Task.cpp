#include "Task.h"



Task::Task(const STask& _task) : task(_task), starting_job_id(-1), ending_job_id(-1) {
	this->set_job_map();
}

Task::Task(int task_id, int release_time, int deadline, int period, int priority) : task{ task_id, release_time, 0, deadline, period, priority, std::map<int, SJob>() }, starting_job_id(-1), ending_job_id(-1)
{
	
}

void Task::set_job_map(const std::vector<SJob>& jobs) {
	this->task.jobs.clear();
	this->starting_job_id = -1;
	this->ending_job_id = -1;

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

	for (std::map<int, SJob>::iterator iter(this->task.jobs.begin()); iter != this->task.jobs.end() && this->starting_job_id == -1; ++iter) {
		if (iter->second.previous_jobs.empty()) this->starting_job_id = iter->second.job_id;
	}
}

void Task::set_job_map() {
	this->starting_job_id = -1;
	this->ending_job_id = -1;
	for (std::map<int, SJob>::iterator iter(this->task.jobs.begin()); iter != this->task.jobs.end(); ++iter) {
		iter->second.previous_jobs.clear();		// recompute previous jobs instead of manual entry
		if (iter->second.next_jobs.empty()) this->ending_job_id = iter->second.job_id;
	}

	for (std::map<int, SJob>::iterator iter(this->task.jobs.begin()); iter != this->task.jobs.end(); ++iter) {
		for (int i = 0; i < iter->second.next_jobs.size(); ++i) {
			this->task.jobs.at(iter->second.next_jobs[i]).previous_jobs.push_back(iter->second.job_id);
		}
	}

	for (std::map<int, SJob>::iterator iter(this->task.jobs.begin()); iter != this->task.jobs.end() && this->starting_job_id == -1; ++iter) {
		if (iter->second.previous_jobs.empty()) this->starting_job_id = iter->second.job_id;
	}
}

void Task::set_job_map(const std::map<int, SJob>& jobs) {
	this->task.jobs = jobs;
	this->set_job_map();
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



















void to_json(nlohmann::json& j, const SJob& job) {
	j = nlohmann::json{
		{
			"job_id", job.job_id
		},
		{
			"execution_time", job.execution_time
		},
		{
			"next_jobs", job.next_jobs
		},
		{
			"previous_jobs", job.previous_jobs
		}
	};
}

void from_json(const nlohmann::json& j, SJob& job) {
	j.at("job_id").get_to(job.job_id);
	j.at("execution_time").get_to(job.execution_time);
	job.next_jobs = j["next_jobs"].get<std::vector<int> >();
	job.previous_jobs = j["previous_jobs"].get<std::vector<int> >();
}

void to_json(nlohmann::json& j, const STask& task) {

	std::vector<SJob> jobs_vec(task.jobs.size());
	int i = 0;
	for (std::map<int, SJob>::const_iterator iter(task.jobs.begin()); iter != task.jobs.end(); ++iter, ++i) jobs_vec[i] = iter->second;

	j = nlohmann::json{
		{
			"task_id", task.task_id
		},
		{
			"release_time", task.release_time
		},
		{
			"deadline", task.deadline
		},
		{
			"period", task.period
		},
		{
			"priority", task.priority
		},
		{
			"jobs", jobs_vec
		}
	};
}

void from_json(const nlohmann::json& j, STask& task) {
	j.at("task_id").get_to(task.task_id);
	j.at("release_time").get_to(task.release_time);
	j.at("deadline").get_to(task.deadline);
	j.at("period").get_to(task.period);
	j.at("priority").get_to(task.priority);
	std::vector<SJob> jobs;
	jobs = j["jobs"].get<std::vector<SJob> >();
	for (int i=0; i < jobs.size(); ++i) task.jobs.insert({ jobs[i].job_id, jobs[i] });
}

void to_json(nlohmann::json& j, const STasks& tasks) {
	j = nlohmann::json{
		{
			"tasks", tasks.tasks
		}
	};
}

void from_json(const nlohmann::json& j, STasks& tasks) {
	tasks.tasks = j["tasks"].get<std::vector<STask> >();
}

