//including the necessary header files
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <fstream>
#include <cmath>
using namespace std;

double context_switch = 0.01;

//a class for a typical process, this has all the details of process
class process_details
{
	//public members and methods
	public:

	int process_id;
	int processing_time;
	int remaining_time;
	int period;
	int number_of_times;
	int repeat_count;
	double entry_time;
	int dead_line;
	int deadline_misses;
	double waiting_time;
	bool is_preempted;

	//constructor for the class
	process_details()
	{
		process_id = 0;
		processing_time = 0;
		remaining_time = 0;
		period = 0;
		entry_time = 0;
		number_of_times = 0;
		repeat_count = 0;
		deadline_misses = 0;
		dead_line = 0;
		waiting_time = 0;
		is_preempted = false;
	}

	//a simple operator overload for checking if two processes are logically equal
	bool operator != (const process_details &newclass)
	{
		return this->process_id != newclass.process_id;
	}
};

//function to print a seperator in the existing file
void print_separator(ofstream &file)
{
	file<<"-----------------------------------------------------------------------------"<<endl;
	file<<"-----------------------------------------------------------------------------"<<endl;
	return;
}

//a class for execution, contains a queue and methods to add processes into queue, check deadline misses and then execute
class schedule_execute
{
	public:

	//a queue for processing queue(standard queue is not used as list offers more functionalities required)
	list<process_details> process_queue;
	//a process for keeping track of last executed task
	process_details last_executed;

	//varibales, real time and cpu flag for cpu idle status
	double real_time;
	int cpuflag;
	//if cpu is idle, cpu start and cpu stop notes the start and end time.
	double cpustart,cpustop;

	//constructor for the class, sets the last executed process's process id as -1
	schedule_execute()
	{
		cpuflag = 0;
		real_time = 0;
		cpustart = 0;
		cpustop = 0;
		last_executed.process_id = -1;
	}

	//method to add process into processing queue
	void add_process(process_details &process,ofstream &file);
	//method to find deadline misses in the processing queue
	void check_deadline_misses(vector<process_details> &details,ofstream &file);
	//method to execute
	void execute(vector<process_details> &details,ofstream &file);
};

//method to add process into processing queue
void schedule_execute::add_process(process_details &process,ofstream &file)
{
	//condition to check if queue is empty
	if(this->process_queue.empty())
	{
		process.entry_time = real_time;
		this->process_queue.push_front(process);
		return;
	}
	//else insert the process into processing queue according to its priority
	else
	{
		int i = 0;
		for(auto it = process_queue.begin() ; it != process_queue.end() ; it++,i++)
		{
			bool preemptive_flag;
			//check the deadlines of coming process and process in the queue
			if(process.dead_line < it->dead_line)
			{
				preemptive_flag = (it->processing_time == it->remaining_time);
				process.entry_time = real_time;
				process_queue.insert(it,process);

				if(preemptive_flag) return;
				else
				{
					if(it->is_preempted)
					;
					//here we get to know the pre-emption status
					else
					{
						it->is_preempted = true;
						file<<"Process P"<<it->process_id<<" is preempted by process P"<<process.process_id<<" at time "<< real_time<<" remaining time:"<<it->remaining_time<<endl;
					}
					return;
				}
			}
			//if the deadlines are equal, the check then process id and assign its place in processing queue
			else if(process.dead_line == it->dead_line)
			{
				if(process.process_id < it->process_id)
				{
					preemptive_flag = (it->processing_time == it->remaining_time);
					process.entry_time = real_time;
					process_queue.insert(it,process);

					if(preemptive_flag) return;
					else
					{
						if(it->is_preempted)
						;
						else
						{
							it->is_preempted = true;
							real_time = (int)real_time;
							file<<"Process P"<<it->process_id<<" is preempted by process P"<<process.process_id<<" at time"<< real_time<<" remaining time:"<<it->remaining_time<<endl;
						}
						return;						
					}
				}
			}
			//if its the last of the queue, then add the coming process to the back of the queue
			if(i == process_queue.size()-1)
			{
				process.entry_time = real_time;
				process_queue.push_back(process);
				return;
			}
		}
	}
}

//method to check deadline misses in the processing queue
void schedule_execute::check_deadline_misses(vector<process_details> &details,ofstream &file)
{
	double sum = context_switch;int it_flag = 0,process_flag = 0;
	list<process_details>::iterator it;

	//checking where the process in the process queue misses its deadline
	for(it = process_queue.begin() ; it != process_queue.end() ; it++)
	{
		//it flag is for getting the respective deadline time at an instance
		it_flag = 1 + (it->repeat_count - it->number_of_times);
		//condition for checking if a process misses it's deadline
		if(sum + real_time + it->remaining_time > it_flag*it->period)
		break;
		sum += context_switch;
	}
	//if no process with deadline miss found, return the control to execute the process in queue
	if(it == process_queue.end())
	return;
	//if processes with deadline misses are found, update the properties in the original container and then remove the processes from the queue
	else
	{
		for(auto &iter: details)
		{
			if(iter.process_id == it->process_id)
			{
				iter.deadline_misses++;
				iter.waiting_time += iter.period;
			}
		}
		file<<"process p"<<it->process_id<<" is going to miss deadline at "<<it_flag*it->period<<endl;
		file<<"removing process p"<<it->process_id<<endl;
		process_queue.erase(it);
		check_deadline_misses(details,file);
	}
	return;
}

//a function to execute the processes one by one in the processing queue
void schedule_execute::execute(vector<process_details> &details,ofstream &file)
{
	//if the process queue is empty, it suggests cpu has no work and thus cpu is idle
	if(process_queue.empty())
	{
		if(!cpuflag)
		{
			cpustart = real_time;
			cpuflag = 1;
		}
		real_time++;
		if(cpuflag)
		{
			cpustop = real_time;
			return;
		}
	}
	//else if cpu is not idle and if there is a difference in start time and stop time, it suggests that
	//cpu just engaged with a process, thus just now cpu completed its idle state
	if(cpustop-cpustart and cpustart != 0 and cpustart != 0)
	{
		cpustop = (int)cpustop;
		real_time = cpustop;
		file <<"cpu is idle for "<<cpustop-cpustart<<"milliseconds from "<<cpustart<<" to "<<cpustop<<" milliseconds"<<endl;
		cpustart = cpustop = cpuflag = 0;
	}

	real_time++;

	//grabbing a process from the front of the processing queue
	process_details &process = process_queue.front();

	process.remaining_time--;

	//flags to check if a process is going to start it's execution
	bool flag1 = (process.remaining_time + 1 == process.processing_time);
	bool flag2 = (process != last_executed and last_executed.process_id != -1);
	if(flag1 or flag2)
	{
		if(process.number_of_times == 0)
		{
			process_queue.pop_front();
			real_time--;
			return;
		}

		real_time += context_switch;
		//status of preempted process is updated in add process function, which helps here
		if(process.is_preempted == false)
		file<<"process p"<<process.process_id<<" has started execution at time "<<real_time-1<<endl;
		else
		{
			file<<"process p"<<process.process_id<<" has resumed it's execution at time "<<real_time-1<<endl;
		}
	}

	//as the process is resumed or started, it's updated as the last executed process for next iteration.
	last_executed = process;

	//if a process remaining time is 0, then it might have completely executed or going to execute completely by a deadline miss
	//here as we removed the missing deadline processes we have only the process which are going to completely executed
	if(process.remaining_time == 0)
	{
		//checking if the process is being completely executed
		if(process.entry_time + process.period >= real_time)
		{
			file<<"process P"<<process.process_id<<" has completely executed at time "<<real_time<<endl;

			double temp = real_time;
			real_time += (process.entry_time - (int)process.entry_time);

			//updating the properties of waiting time in original details
			for(auto &it:details)
			{
				if(it.process_id == process.process_id)
				{
					it.waiting_time += real_time-process.entry_time-process.processing_time;
				}
			}
			real_time = temp;
			//as the process is completely executed, let's pop it from the queue
			process_queue.pop_front();
			return;
		}
	}

}

//a function to sort the details in order of increasing priority
bool comparision(const process_details &process1,const process_details &process2)
{
	return process1.period < process2.period;
}

//printing joining status for the processes
void print_join_status(vector<process_details> details,int number_of_processes,ofstream &file)
{
	file<<"Total number of processes : "<<number_of_processes<<" with unique process-id"<<endl;
	for(int i = 0 ; i < number_of_processes ; i++)
	{
		file<<"Process P"<<details[i].process_id<<": ";
		file<<"processing time="<<details[i].processing_time<<"; ";
		file<<"period:"<<details[i].period<<"; ";
		file<<"joined the system at time 0"<<endl;
	}
}

//function for schedulabilty to check beforehand the processes begin to execute by the formulae
bool schedulability(vector<process_details>details,int number_of_processes,ofstream &file)
{
	double tempvar1 = 0,tempvar2 = 1;
	for(int i = 0 ; i < number_of_processes ; i++)
	{
		tempvar1 = tempvar1 + double(details[i].processing_time)/(details[i].period);
	}
	file<<"Estimated cpu utilization for one round:"<<tempvar1*100<<"%"<<endl;
	if(tempvar1 < tempvar2)
	file<<"The scheduling of processes is expected to complete without deadline miss!"<<endl;
	else
	file<<"The scheduling of processes is expected to have a deadline miss!"<<endl;
	print_separator(file);
	return true;
}


//earliest deadline first scheduler
void earliest_deadline_first_scheduler(vector<process_details> &details,int number_of_processes)
{
	ofstream outlogfile("EDF-log.txt");
	outlogfile<<"Scheduling Log for the input processes"<<endl;

	//printing the separator
	print_separator(outlogfile);

	//print the join status of all the processes
	print_join_status(details,number_of_processes,outlogfile);

	//printing the separator
	print_separator(outlogfile);

	//sorting the process in order of priority
	sort(details.begin(),details.end(),comparision);

	//information about schedulability, anyways it is going to be executed, estimating the deadline misses and cpu utilization
	bool is_schedulable = schedulability(details,number_of_processes,outlogfile);

	//creating an instance of class for scheduling
	class schedule_execute schedule_processes;

	//starting the scheduler
	if(is_schedulable)
	{


		//real time loop
		for(int i = 0 ; ; i++)
		{
			//a loop which adds the processes into the processing queue and updates the count of number of times it should repeat.
			for(auto &process : details)
			{
				if(i%(process.period) == 0 and process.number_of_times > 0)
				{
					process.dead_line += process.period;
					schedule_processes.add_process(process,outlogfile);
					process.number_of_times--;
				}
			}

			//condition for aborting the scheduler, else it waits all long for incoming processes and never terminate
			//if a process queue is empty and if all the repeat counts of the processes reaches 0, then there are no new processes
			//and thus terminating the scheduler
			int process_flag = 0;
			if(schedule_processes.process_queue.empty())
			{
				//checking if a process has to execute atleast once more
				for(auto it:details)
				{
					if(it.number_of_times != 0)
					{
						process_flag = 1;
						break;
					}
				}
				if(process_flag == 0)
				break;
			}

			//calling the check deadline miss function
			schedule_processes.check_deadline_misses(details,outlogfile);
			//calling the execute function
			schedule_processes.execute(details,outlogfile);
		}
	}
	double cpustop = schedule_processes.cpustop-1;
	double cpustart = schedule_processes.cpustart;
	if(cpustop - cpustart and cpustop != 0 and cpustart != 0)
	{
		cpustop = (int)cpustop;
		outlogfile <<"cpu is idle for "<<cpustop-cpustart<<"milliseconds from "<<cpustart<<" to "<<cpustop<<" milliseconds"<<endl;
	}
	//separator for clear distinction between blocks
	print_separator(outlogfile);
	outlogfile<<"Execution completed"<<endl;
	print_separator(outlogfile);
}

//function to print statistics after complete scheduling of the processes
void print_stats(vector<process_details> &details,int number_of_processes)
{
	//opening a new file and adding the titles
	ofstream outstatsfile("EDF-stats.txt");
	outstatsfile<<"Statistics for the input processes"<<endl;

	int successful_count = 0,deadline_missed_count = 0;
	int sum_of_successful=0;
	//checking if there are deadline misses encountered and analysing the statistics
	for(auto it: details)
	{
		if(it.deadline_misses)
		{
			deadline_missed_count += it.deadline_misses;
			sum_of_successful += it.repeat_count-it.deadline_misses;
		}
		else
		{
			successful_count++;
			sum_of_successful += it.repeat_count;
		}
	}
	print_separator(outstatsfile);
	outstatsfile<<"Number of processes that came into system : "<<deadline_missed_count+sum_of_successful<<endl;
	print_separator(outstatsfile);
	if(successful_count == number_of_processes)
	{
		outstatsfile<<"Every process is successfully executed without a deadline miss!"<<endl;
		outstatsfile<<"Successfully executed processes without a deadline miss : "<<sum_of_successful<<endl;
		print_separator(outstatsfile);
		outstatsfile<<"No deadline miss encountered in the given set of processes!"<<endl;
		outstatsfile<<"Number of deadline misses : "<<deadline_missed_count<<endl;
		print_separator(outstatsfile);
	}
	else
	{
		outstatsfile<<"Some processes missed their deadline"<<endl;
		outstatsfile<<"Number of successfully executed processes without a deadline miss : "<<sum_of_successful<<endl;
		print_separator(outstatsfile);
		outstatsfile<<"Deadline misses are encountered!"<<endl;
		outstatsfile<<"Number of processes that missed it's deadline : "<<deadline_missed_count<<endl;
		outstatsfile<<"the processes are :"<<endl;

		for(auto it : details)
		{
			if(it.deadline_misses == 1)
			outstatsfile<<"process with process-id:"<<it.process_id<<" missed it's deadline 1 time"<<endl;
			else if(it.deadline_misses > 1)
			outstatsfile<<"process with process-id:"<<it.process_id<<" missed it's deadline "<<it.deadline_misses<<" times"<<endl;
		}
		print_separator(outstatsfile);
	}
	//for getting the overall waiting time, average waiting time for each process and overall processes
	double sum = 0;
	for(auto it : details)
	{
		outstatsfile<<"The overall waiting time for the process P"<<it.process_id<<" : "<<it.waiting_time<<endl;
		outstatsfile<<"Average waiting time for the process P"<<it.process_id<<" : "<<(double)it.waiting_time/it.repeat_count<<endl;
		sum += (double)it.waiting_time/it.repeat_count;
	}
	outstatsfile<<endl<<"Average waiting time for all the processes : "<<(double)sum/number_of_processes<<endl;
	print_separator(outstatsfile);
}

//main function
int main()
{
	//input file
	ifstream inputfile("inp-params.txt");

	//checking if the file exists and readable 
	if(inputfile)
	{
		//fetching data number of processes
		int number_of_processes;
		inputfile>>number_of_processes;

		//for list of detailed information of processes
		process_details* details = new process_details[number_of_processes];

		//container for the processes
		vector<process_details> processes;

		//fetching the data for all the processes.
		for(int i = 0 ; i < number_of_processes ; i++)
		{
			inputfile>>details[i].process_id;
			inputfile>>details[i].processing_time;
			inputfile>>details[i].period;
			inputfile>>details[i].number_of_times;
			details[i].remaining_time = details[i].processing_time;
			details[i].repeat_count = details[i].number_of_times;
			processes.push_back(details[i]);
		}

		//calling the rate monotonic scheduler on the processes fetched
		earliest_deadline_first_scheduler(processes,number_of_processes);

		//function for printing the statistics of the given process
		print_stats(processes,number_of_processes);
	}
	//if the file doesn't exists or unable to read
	else
	{
		cout<<"Error in reading the input file!"<<endl;
	}
	return 0;
}