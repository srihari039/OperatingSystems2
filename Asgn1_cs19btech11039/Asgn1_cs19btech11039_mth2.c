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
	long begin;
	long end;
};

//a structure for all the properties required for computation
struct properties
{
	long *array;
	long count;
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
	long mid = -1 + second + (first - second)/2;

	long i = first,j = mid+1,k = 0;

	while(i <= mid && j <= second)
	{
		if(array[i] < array[j])
		temp_array[k++] = array[i++];
		else
		temp_array[k++] = array[j++];
	}

	while(i <= mid)
	temp_array[k++] = array[i++];
	while(j <= second)
	temp_array[k++] = array[j++];

	for(i = first,j = 0;i <= second ; i++,j++)
	array[i] = temp_array[j];

	free(temp_array);
} 

void* merge_array(void* thread)
{
	struct properties* sort_using_threads = (struct properties*)thread;
	long count = sort_using_threads->count++;
	long low = sort_using_threads->merge_helper[count].begin,high = sort_using_threads->merge_helper[count].end;

	merge(sort_using_threads->array,low,high-1);	
	return NULL;
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
		long i,n,p,no_of_threads,size_of_array,reserved,size_of_segment,k = 0,base = 1,error,phase_count = 0;
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
			
			error = pthread_create(&tid_segmentsorting[i],NULL,quicksortarray,(void*)sort_using_threads);
			//if there exists error in thread creation
			if(error)
			printf("Thread-%ld creation failed with return value-%ld in segment sorting\n",i,error);
		}

		//joining all threads(sorted segments)
		for(i = 0 ; i < no_of_threads ; i++)
		pthread_join(tid_segmentsorting[i],NULL);
		//all the segments get sorted here

		long threads = no_of_threads;

		//thread for merging the segments as per method 2
		for( ; threads > 1 ; threads /= 2)
		{
			i = 0;
			base = base*2;
			long initial = 0,final;
			for( ; i < size_of_array ; )
			{
				final = initial + base*(reserved+1);
				sort_using_threads->merge_helper[k].begin = initial;
				sort_using_threads->merge_helper[k++].end = final;
				i = final;
				initial = final;
			}
		}
		sort_using_threads->count = 0;

		//creation and joining of threads as per method2
		no_of_threads /= 2;
		while(no_of_threads)
		{
			phase_count++;
			pthread_t tid_merge[no_of_threads];

			for(i = 0 ; i < no_of_threads ; i++)
			{
				error = pthread_create(&tid_merge[i],NULL,merge_array,(void*)sort_using_threads);
				if(error)
				printf("Thread-%ld creation failed with return value-%ld in merging phase-%ld\n",i,error,phase_count);
			}
			for(i = 0 ; i < no_of_threads ; i++)
			pthread_join(tid_merge[i],NULL);

			no_of_threads /= 2;
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