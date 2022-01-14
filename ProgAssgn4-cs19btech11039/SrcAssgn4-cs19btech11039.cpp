//including required header files
#include <iostream>
#include <fstream>
#include <thread>
#include <semaphore.h>
#include <climits>
#include <random>
#include <chrono>
#include <sys/time.h>
using namespace std;

//global declarations
//for number of customers, size of the table, number of people eating and waiting, for lambda r and tou
int number_of_customers = 0,size_of_table = 0,eating = 0,waiting = 0,temp = INT_MAX,time_var = 0;
double lambda,r,tou;

//ofstream file for writing
ofstream outputfile("log.txt");

//declarations of threads 
thread* worker_threads;

//declaration of semaphores
sem_t mutex,block,binary;

//a boolean variable for utility
bool must_wait = false;

//declarations of timeval quantities to record time stamps
struct timeval *request,*access,*completed,initial;
time_t pres = time(NULL);
tm *pres_req,*pres_acc,*pres_com;

//exit restaurant definition
void exit_restaurant(int);

//function that takes a double value, and gives an exponentially distributed random number
double exponential(double value)
{
	exponential_distribution<double> distribution(value);
	random_device randomdevice;
	default_random_engine generate(randomdevice());
	return distribution(generate);
}

//function that takes a double value and gives an uniformly distributed random number from 1 to value
double uniform(double value)
{
	uniform_real_distribution<double> distribution(1,value);
	random_device randomdevice;
	default_random_engine generate(randomdevice());
	return distribution(generate);
}

//function that calculates time gap between two time stamps
double calculate_time_gap(char c,int i)
{
	if(c == 'r')
	return (request[i].tv_sec-initial.tv_sec)*1e6 + (request[i].tv_usec-initial.tv_usec);
	else if(c == 'a')
	return (access[i].tv_sec-initial.tv_sec)*1e6 + (access[i].tv_usec-initial.tv_usec);
	else
	return (completed[i].tv_sec-initial.tv_sec)*1e6 + (completed[i].tv_usec-initial.tv_usec);
}

//function that simulates the entring of restaurant
void enter_restaurant(int th_no)
{
	//wait upon a mutex
	sem_wait(&mutex);

	//wait upon a binary semaphore and record time stamps
	sem_wait(&binary);
	pres_req = localtime(&pres);
	gettimeofday(&request[th_no],NULL);
	time_var = calculate_time_gap('r',th_no);
	outputfile<<th_no<<"th customer access request at "<<pres_req->tm_hour<<":"<<pres_req->tm_min<<":"<<pres_req->tm_sec+time_var/1e6<<endl;
	//signalling the binary semaphore
	sem_post(&binary);

	//condition to ensure the remaining threads comes here and waits till signalled
	if(must_wait == true or eating + 1 > size_of_table)
	{
		//incrementing the count of waiting
		waiting++;
		//setting the must wait boolean to true
		must_wait = true;
		//signalling mutex
		sem_post(&mutex);
		//waiting on block
		sem_wait(&block);

		//now threads unvieled from here enters directly the eating state, given access, so theres need to record the time stamp
		//wait upon a binary semaphore and record the respective time stamp
		sem_wait(&binary);
		pres_acc = localtime(&pres);
		gettimeofday(&access[th_no],NULL);
		time_var = calculate_time_gap('a',th_no);
		outputfile<<th_no<<"th customer given access at "<<pres_acc->tm_hour<<":"<<pres_acc->tm_min<<":"<<pres_acc->tm_sec+time_var/1e6<<endl;
		//signalling the binary semaphore
		sem_post(&binary);
	}
	//no waiting
	else
	{
		//as already in mutex, it is thread safe, record the respective time stamp
		pres_acc = localtime(&pres);
		gettimeofday(&access[th_no],NULL);
		time_var = calculate_time_gap('a',th_no);
		sem_wait(&binary);
		outputfile<<th_no<<"th customer given access at "<<pres_acc->tm_hour<<":"<<pres_acc->tm_min<<":"<<pres_acc->tm_sec+time_var/1e6<<endl;
		sem_post(&binary);
		//increasing the eating count
		eating++;
		//setting the boolean value
		must_wait = (waiting > 0 and eating == size_of_table);
		//signalling the mutex
		sem_post(&mutex);
	}

	//getting a random exponential time, for which the thread sleeps
	int time = exponential(tou);
	sem_wait(&binary);
	outputfile<<"Eating time for customer-"<<th_no<<" "<<time<<" milli seconds"<<endl;
	sem_post(&binary);
	this_thread::sleep_for(chrono::milliseconds(time));

	//function to exit restaurant
	exit_restaurant(th_no);

	//after the exit restaurant returns the control, time to record exit stamps
	//waiting on a binary semaphore
	sem_wait(&binary);
	pres_com = localtime(&pres);
	gettimeofday(&completed[th_no],NULL);
	time_var = calculate_time_gap('c',th_no);
	outputfile<<th_no<<"th customer exits at "<<pres_com->tm_hour<<":"<<pres_com->tm_min<<":"<<pres_com->tm_sec+time_var/1e6<<endl;
	//signalling the binary semaphore
	sem_post(&binary);
}

//function that simulates exiting restaurant
void exit_restaurant(int th_no)
{
	//wait on a mutex
	sem_wait(&mutex);
	//decreasing the eating count
	eating--;
	//if present eating equals to 0, then update the properties
	if(eating == 0)
	{
		//getting the minimum of size of table and number of people waiting
		temp = std::min(size_of_table,waiting);
		//updating the waiting count
		waiting -= temp;
		//updating the eating count
		eating += temp;
		//setting the boolean flag
		must_wait = (eating == size_of_table and waiting > 0);
		//signal the block temp times, such that temp number of threads comes to eating state
		for(int i = 0 ; i < temp ; i++)
		sem_post(&block);
	}
	//signalling the mutex
	sem_post(&mutex);
}

//main function
int main()
{
	//file handling
	string inputfilename = "input.txt";
	ifstream openfile(inputfilename);
	//fetching inputs
	openfile>>number_of_customers>>size_of_table>>lambda>>r>>tou;
	//converting to their inverse for average
	tou = 1/tou; lambda = 1/lambda;

	//allocating threads
	worker_threads = new thread[number_of_customers];

	//initialising the semaphores
	sem_init(&mutex,0,1);
	sem_init(&block,0,0);
	sem_init(&binary,0,1);

	//variables for proper ensurance of handling threads
	int temp = number_of_customers;
	int set = 0, i = 0;

	//allocating the timeval quantities
	request = new timeval[number_of_customers];
	access = new timeval[number_of_customers];
	completed = new timeval[number_of_customers];

	//a while loop in which thread management is done
	while(temp)
	{
		//gets a value
		set += uniform(r*size_of_table);
		if(i == 0)
		gettimeofday(&initial,NULL);

		//a for loop for looping up the order and creating new threads
		for( ; i < set and i < number_of_customers ; i++)
		worker_threads[i] = thread(enter_restaurant,i);
		
		//reducing the number of people yet to join the table
		temp = number_of_customers-i;

		//getting an exponential time for the main thread to sleep on
		int time = exponential(lambda);
		this_thread::sleep_for(chrono::milliseconds(time));
	}

	//joining all the threads
	for(int i = 0 ; i < number_of_customers ; i++)
	worker_threads[i].join();

	//destroying the semaphores
	sem_destroy(&mutex);
	sem_destroy(&block);
	sem_destroy(&binary);

	//destroying the entities allocated
	delete[] worker_threads;
	delete[] request;
	delete[] access;
	delete[] completed;

	//exit success
	return 0;
}
