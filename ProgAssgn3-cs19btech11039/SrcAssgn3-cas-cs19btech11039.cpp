//including header files
#include <iostream>
#include <thread>
#include <fstream>
#include <random>
#include <unistd.h>
#include <time.h>
#include <atomic>
#include <cmath>
using namespace std;

//atomic bool variable
atomic_bool flag_var;
bool var_t = false;

//global variables for number of threads, repetition count and lambdas
int number_of_threads,repetition_count;
double lambda1,lambda2;
ofstream file("cas-output.txt");

//entry section simulation
void entry_sec()
{
	while(atomic_compare_exchange_strong(&flag_var,&var_t,true));
	return;
}

//exit section simulation
void exit_sec()
{
	if(flag_var)
	flag_var = false;
	return;
}

//using in built functions to get a random real time simulation time
double exponential_mimic(double lambda)
{
	uniform_real_distribution<double> distribution(pow(lambda,-1));
	random_device randomdevice;
	default_random_engine generate(randomdevice());
	return distribution(generate);
}

//function to get time in milli seconds
int get_milliseconds()
{
	using namespace std::chrono;
	auto now = system_clock::now();
	auto ms = duration_cast<milliseconds>(now.time_since_epoch())%1000;
	return ms.count();
}

//the test Critical Section function
void testCS(int id)
{
	//a for loop for repeated entry of a thread
	for(int i = 0 ; i < repetition_count ; i++)
	{
		//time stamps for request time, entry time and exit time
		time_t cs_req_time, cs_entry_time, cs_exit_time;
	
		//get request time
		cs_req_time = time(NULL);
		tm* ltm = localtime(&cs_req_time);

		//printing the request time stamp
		file<<i<<"th cs request at time: "<<ltm->tm_hour<<":"<<ltm->tm_min<<":"<<ltm->tm_sec<<":"<<get_milliseconds()<<" by thread"<<id<<endl;

		//entry section
		entry_sec();

		//get entry time
		cs_entry_time = time(NULL);
		ltm = localtime(&cs_entry_time);

		//printing the entry time stamp
		file<<i<<"th cs entry at time: "<<ltm->tm_hour<<":"<<ltm->tm_min<<":"<<ltm->tm_sec<<":"<<get_milliseconds()<<" by thread"<<id<<endl;

		//mimicing the functionality of critical section
		sleep(exponential_mimic(lambda1));

		//get exit time
		cs_exit_time = time(NULL);
		ltm = localtime(&cs_exit_time);
		file<<i<<"th cs exit at time: "<<ltm->tm_hour<<":"<<ltm->tm_min<<":"<<ltm->tm_sec<<":"<<get_milliseconds()<<" by thread"<<id<<endl;

		//exit section
		exit_sec();

		sleep(exponential_mimic(lambda2));
	}
}

//function to create new threads of desired size
thread* create_threads(int size)
{
	thread* new_threads = new thread[size];
	return new_threads;
}

int main()
{
	//threads for entering Critical Section
	thread* worker_threads;

	//opening the file "inp-params.txt"
	ifstream file_to_be_opened("inp-params.txt");

	//if it is openable without any errors
	if(file_to_be_opened.is_open())
	{
		//fetching the inputs for number of threads, repetition count and the lambdas
		file_to_be_opened>>number_of_threads>>repetition_count>>lambda1>>lambda2;
		
		//creating new threads
		worker_threads = create_threads(number_of_threads);
		
		//calling the thread functions in a loop
		for(int i = 0 ; i < number_of_threads ; i++)
		worker_threads[i] = thread(testCS,i+1);

		//calling the join on all the threads
		for(int i = 0 ; i < number_of_threads ; i++)
		worker_threads[i].join();

		//deleting the threads created
		delete[] worker_threads;
	}
	//if it is not openable, prints an error message
	else
	cout<<"Failed to open file \"inp-params.txt\""<<endl;

	return 0;
}