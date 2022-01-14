//including header files for utility
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/wait.h>

//structure for splitting array into segments
struct splitarray
{
	long start;
	long end;
};

//structure for dealing with merging the segments
struct mergearray
{
	long max;
};

//a structure for all the properties required for computation
struct properties
{
	long *array;
	long count;
	long segment_size;
	struct splitarray *segmentsort_helper;
	struct mergearray *merge_helper;
};

//utility function for populating a random array
void populatearray(long* array,long n)
{
	for(long i = 0 ; i < n ; i++)
	{
		array[i] = rand();
	}
	return;
}

//function for printing the arrays into desired file
void printarray(FILE* file,long *array,long n,bool status)
{
	for(long i = 0 ; i < n ; i++)
	{
		fprintf(file,"%ld ",array[i]);
	}
	if(status)
	fprintf(file,"//Array after sorting\n");
	else
	fprintf(file,"//Array before sorting\n");

	return;
}

//utility function for swapping integers in array
void swap(long* a, long* b) 
{ 
	long t = *a; 
	*a = *b; 
	*b = t; 
}

//partition function for quick sort
long partition(long arr[], long low, long high)
{
	long i = low - 1;
	long j = high + 1;
	long pivot = arr[low];

	for( ; ; )
	{
		i++,j--;

		while (arr[i] < pivot)
		i++; 

		while (arr[j] > pivot)
		j--;
 
		if (i >= j)
		return j;

		swap(&arr[i], &arr[j]);
	}
}

//quicksort function for sorting the array
void quicksort(long arr[], long low, long high)
{
	if (low < high)
	{
		long random = low + rand() % (high - low);
		swap(&arr[random], &arr[low]);
		long pi = partition(arr, low, high);
		quicksort(arr, low, pi);
		quicksort(arr, pi + 1, high);
	}
}

//quicksort caller function(thread)
void* quicksortarray(void* thread)
{
	//getting the arguments and loading
	struct properties* sort_using_threads = (struct properties*)thread;

	//for thread instance
	long count = sort_using_threads->count++;

	//calling quicksort function
	quicksort(sort_using_threads->array,sort_using_threads->segmentsort_helper[count].start,sort_using_threads->segmentsort_helper[count].end);

	return NULL;
}

//function to merge two sorted arrays in time O(p+q) ,p and q are the size of arrays
void merge(long* array,long first,long second)
{
	long* temp_array = (long*)malloc(second*sizeof(long)+1024);
	long i = 0,j = first+1,k = 0;

	while(i <= first && j <= second)
	{
		if(array[i] < array[j])
		temp_array[k++] = array[i++];
		else
		temp_array[k++] = array[j++];
	}

	while(i <= first)
	temp_array[k++] = array[i++];
	while(j <=  second)
	temp_array[k++] = array[j++];

	for(long i = 0 ; i < k ; i++)
	array[i] = temp_array[i];

	free(temp_array);
} 

//function to merge the sorted segments
void merge_array(long *array,long high,long segment_size)
{
	merge(array,high-segment_size,high);
}

//Main function
int main()
{
	//seeding the time for random values
	srand(time(NULL));

	//file operations to load the input values
	FILE* filetobeopened;
	filetobeopened = fopen("inp.txt","r");

	//checking if input file exists
	if(filetobeopened)
	{
		//declaring required variables
		long i,n,p,no_of_threads,size_of_array,reserved,size_of_segment,error;
		fscanf(filetobeopened,"%ld %ld",&n,&p);

		//getting the size of array and no of threads required
		size_of_array = pow(2,n);
		no_of_threads = pow(2,p);
		reserved = size_of_array/no_of_threads - 1;
		size_of_segment = reserved+1;

		//allocating memory for the structure to store properties of computation
		struct properties* sort_using_threads = (struct properties*)malloc(sizeof(struct properties));

		//allocating the memory for array and populating the array with random variables
		sort_using_threads->array = (long*)malloc(size_of_array*sizeof(long));
		populatearray(sort_using_threads->array,size_of_array);

		//for utility and keeping track of thread
		sort_using_threads->count = 0;
		sort_using_threads->segment_size = size_of_segment;

		//declaring thread id's for segment sorting
		pthread_t tid_segmentsorting[no_of_threads];

		//allocating memory for holding various properties for threads to work on
		sort_using_threads->segmentsort_helper = (struct splitarray*)malloc(no_of_threads*(sizeof(struct splitarray)));
		sort_using_threads->merge_helper = (struct mergearray*)malloc((no_of_threads-1)*sizeof(struct mergearray));

		//opening the file to be written
		FILE* filetobewritten;
		filetobewritten = fopen("Output.txt","w");

		//printing array before sorting
		printarray(filetobewritten,sort_using_threads->array,size_of_array,false);

		//timeval for recording timestamps
		struct timeval start,end;

		//recording the timestamp at start of all operations
		gettimeofday(&start,NULL);

		//creating threads -- segments for sorting
		for(i = 0 ; i < no_of_threads ; i++)
		{
			if(i == 0)
			sort_using_threads->segmentsort_helper[i].start = 0;			
			else
			sort_using_threads->segmentsort_helper[i].start = sort_using_threads->segmentsort_helper[i-1].end + 1;
			
			sort_using_threads->segmentsort_helper[i].end = sort_using_threads->segmentsort_helper[i].start + reserved;
			sort_using_threads->merge_helper[i-1].max = sort_using_threads->segmentsort_helper[i].end;
			
			error = pthread_create(&tid_segmentsorting[i],NULL,quicksortarray,(void*)sort_using_threads);
			//if there exists error in thread creation
			if(error)
			printf("Thread-%ld creation failed with return value-%ld\n",i,error);
		}

		//joining all threads(sorted segments)
		for(i = 0 ; i < no_of_threads ; i++)
		pthread_join(tid_segmentsorting[i],NULL);
		//all the segments get sorted here


		//main thread managing merging the segments as per method 1
		for(long i = 0 ; i < no_of_threads - 1 ; i++)
		{
			//it gives arguments according to method1
			merge_array(sort_using_threads->array,sort_using_threads->merge_helper[i].max,size_of_segment);
		}
		//after end of all phases i.e no_of_threads - 1 phases, the array gets sorted completely. 

		//recording the time stamp after all operations
		gettimeofday(&end,NULL);

		//printing array after sorting
		printarray(filetobewritten,sort_using_threads->array,size_of_array,true);

		//calculating time taken
		double timetaken = (end.tv_sec - start.tv_sec) * 1e6;
		timetaken = (timetaken + (end.tv_usec - start.tv_usec)) * 1e-6;

		//printing the elapsed time
		fprintf(filetobewritten,"elapsed time: %0.6lf sec\n",timetaken);

		//closing all opened files
		fclose(filetobewritten);
		fclose(filetobeopened);
	}
	else
	printf("Error opening the file!\n");

	//exiting the thread notion
	pthread_exit(NULL);
	return 0;
}