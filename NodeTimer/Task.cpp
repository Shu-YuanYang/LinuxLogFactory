#include "Task.h"
#include <queue>


Task::Task(const STask& _task) : task(_task), starting_job_id(-1), ending_job_id(-1), eligible_job_refs() {
	this->set_job_map();
}

Task::Task(int task_id, int release_time, int deadline, int period, int priority) : task{ task_id, release_time, 0, deadline, period, priority, std::map<int, SJob>() }, starting_job_id(-1), ending_job_id(-1), eligible_job_refs()
{
	
}

void Task::set_job_map(const std::vector<SJob>& jobs) {
	this->task.jobs.clear();
	this->starting_job_id = -1;
	this->ending_job_id = -1;

	for (std::vector<SJob>::const_iterator iter(jobs.begin()); iter != jobs.end(); ++iter) {
		auto result = this->task.jobs.insert({ iter->job_id, *iter });
		result.first->second.previous_jobs.clear();		// recompute previous jobs instead of manual entry
		result.first->second.deadline = this->task.deadline;
		//result.first->second.progress = 0;
		//result.first->second.__eligibility_index__ = 0;
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
		if (iter->second.previous_jobs.empty()) { 
			this->starting_job_id = iter->second.job_id; 
			//eligible_job_refs.insert({ this->starting_job_id, this->task.release_time + iter->second.deadline });
		}
	}
	

	this->compute_job_deadlines();
	this->renew_period(this->task.release_time);
}

void Task::set_job_map() {
	this->starting_job_id = -1;
	this->ending_job_id = -1;
	for (std::map<int, SJob>::iterator iter(this->task.jobs.begin()); iter != this->task.jobs.end(); ++iter) {
		iter->second.previous_jobs.clear();		// recompute previous jobs instead of manual entry
		iter->second.deadline = this->task.deadline;
		//iter->second.progress = 0;
		//iter->second.__eligibility_index__ = 0;
		if (iter->second.next_jobs.empty()) this->ending_job_id = iter->second.job_id;
	}

	for (std::map<int, SJob>::iterator iter(this->task.jobs.begin()); iter != this->task.jobs.end(); ++iter) {
		for (int i = 0; i < iter->second.next_jobs.size(); ++i) {
			this->task.jobs.at(iter->second.next_jobs[i]).previous_jobs.push_back(iter->second.job_id);
		}
	}

	for (std::map<int, SJob>::iterator iter(this->task.jobs.begin()); iter != this->task.jobs.end() && this->starting_job_id == -1; ++iter) {
		if (iter->second.previous_jobs.empty()) { 
			this->starting_job_id = iter->second.job_id; 
			//eligible_job_refs.insert({ this->starting_job_id, this->task.release_time + iter->second.deadline });
		}
	}

	this->compute_job_deadlines();
	this->renew_period(this->task.release_time);
}

void Task::set_job_map(const std::map<int, SJob>& jobs) {
	this->task.jobs = jobs;
	this->set_job_map();
}



struct __deadline_order__ {
	int job_id;
	int deadline;
};

void Task::compute_job_deadlines() {
	// reverse lookup
	std::queue<__deadline_order__> previous_job_deadline_orders;
	SJob* current_job = &(this->task.jobs.at(this->ending_job_id));
	current_job->deadline = this->task.deadline;
	for (int i = 0; i < current_job->previous_jobs.size(); ++i) previous_job_deadline_orders.push(__deadline_order__{ current_job->previous_jobs[i], current_job->deadline - current_job->execution_time });
	
	while (!previous_job_deadline_orders.empty()) {
		const __deadline_order__& order = previous_job_deadline_orders.front();
		current_job = &(this->task.jobs[order.job_id]);
		current_job->deadline = std::min(current_job->deadline, order.deadline);
		previous_job_deadline_orders.pop();
		
		for (int i = 0; i < current_job->previous_jobs.size(); ++i) previous_job_deadline_orders.push(__deadline_order__{ current_job->previous_jobs[i], current_job->deadline - current_job->execution_time });
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

int Task::get_job_laxity(int job_id, int current_time_step) const {
	int absolute_deadline = this->eligible_job_refs.at(job_id);
	const SJob& job_ref = this->task.jobs.at(job_id);
	return (absolute_deadline - current_time_step) - (job_ref.execution_time - job_ref.progress);
}

int Task::get_total_execution_time() const {
	int total_execution_timer = 0;
	for (std::map<int, SJob>::const_iterator iter(this->task.jobs.begin()); iter != this->task.jobs.end(); ++iter) {
		total_execution_timer += iter->second.execution_time;
	}
	return total_execution_timer;
}


const std::map<int, int>& Task::get_eligible_absolute_deadlines() const {
	return this->eligible_job_refs;
}

void Task::update_job_progress(int job_id, int time_units) {
	std::map<int, int>::iterator iter(this->eligible_job_refs.find(job_id));
	if (iter == this->eligible_job_refs.end()) throw "Job ID: " + std::to_string(job_id) + " is not eligible for execution!";
	SJob& job = this->task.jobs.at(job_id);
	job.progress += time_units;
	if (job.execution_time < job.progress) throw "Progress of Job ID: " + std::to_string(job_id) + " is " + std::to_string(job.progress) + " and exceeds the allocated execution time " + std::to_string(job.execution_time);
	if (job.execution_time != job.progress) return;	// no state changes for eligible jobs
	this->eligible_job_refs.erase(iter);
	for (int i = 0; i < job.next_jobs.size(); ++i) {
		SJob& next_job = this->task.jobs.at(job.next_jobs[i]);
		++next_job.__eligibility_index__;
		if (next_job.__eligibility_index__ == next_job.previous_jobs.size())
			this->eligible_job_refs.insert({ next_job.job_id, this->task.release_time + next_job.deadline });
	}
}


void Task::renew_period(int current_time_step) {
	if ((current_time_step - this->task.release_time) % this->task.period != 0) throw "Period has not ended for Task ID: " + std::to_string(this->task.task_id) + " with release time = " + std::to_string(this->task.release_time) + " and period = " + std::to_string(this->task.period) + " at time step " + std::to_string(current_time_step);
	this->eligible_job_refs.clear();
	this->task.release_time = current_time_step; // update release time
	for (std::map<int, SJob>::iterator iter(this->task.jobs.begin()); iter != this->task.jobs.end(); ++iter) {
		iter->second.progress = 0;
		iter->second.__eligibility_index__ = 0;
	}
	SJob& starting_job = this->task.jobs.at(this->starting_job_id);
	this->eligible_job_refs.insert({ this->starting_job_id, this->task.release_time + starting_job.deadline });
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
			"deadline", job.deadline
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

