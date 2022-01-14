//including the header files for utility
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <set>
#include <mutex>
#include <sys/time.h>
#include <sstream>
using namespace std;

//global variables for utility
int number_of_threads,number_of_vertices,temp;//,colour = 1;
//declaring threads
thread* worker_threads;
//for storing colours for vertices
unordered_map<int,int> colours;
//lock for achieving successful coarse grained algorithm
pthread_mutex_t mutexlock;

//class for graph opetations
class Graph_colouring
{
	private:

		//sub class for storing the random partitions
		class init_parts
		{

			public:

			//start and end variables,
			int start;
			int end;
			
			//constructor
			init_parts();
			
			// getter and setter methods
			void set(int st, int en);
			int getst();
			int geten();
		};

	public:

	//vector for randomising vertices
	vector<int> vertices;
	//array of the partition attribute
	init_parts* parts;
	//a status vector for having the coloured status of the vertex
	vector<int> status;
	//a matrix for the graph
	vector<vector<int>> graph;

	//constructor
	Graph_colouring(vector<vector<int>>);
	//destructor
	~Graph_colouring();

	//method to split the parts
	void split();

};

//construtor for the sub class init parts
Graph_colouring::init_parts::init_parts()
{
	start = 0;
	end = 0;
}

//setter method to set the start and end to the partition
void Graph_colouring::init_parts::set(int st,int en)
{
	start = st;
	end = en;
	return;
}

//getter method to get the start of the partition
int Graph_colouring::init_parts::getst()
{
	return start;
}

//getter method to get the end of the partition
int Graph_colouring::init_parts::geten()
{
	return end;
}

//function to randomise the vertices
void randomise(vector<int> &vertices)
{
	//seeding the time
	srand(time(NULL));

	//swapping the vertices randomly in the vertices array
	int length = vertices.size()/2,i;
	for(i = 0 ; i < length ; i++)
	{
		//using in-built swap function to swap the elements in the vector
		swap(vertices[i],vertices[rand()%(number_of_vertices)]);
	}
}

//constructor for the class graph colouring
Graph_colouring::Graph_colouring(vector<vector<int>> new_graph)
{
	//assign the graph
	this->graph = new_graph;
	//allocate memory to the partition to store
	this->parts = new init_parts[number_of_vertices];
	//resizing and filling the status vector
	status.resize(number_of_vertices);
	fill(status.begin(),status.end(),0);
	//filling the vertices and randomising the vertices
	for(int i = 0 ; i < number_of_vertices ; i++)
	vertices.push_back(i);
	randomise(vertices);
}

//destructor for the class
Graph_colouring::~Graph_colouring()
{
	//frees the allocated memory
	delete[] parts;
}

//method to split the partitions
void Graph_colouring::split()
{
	//already the vertices array gets randomised
	//here we only assign a starting index and ending index for each partition
	int temp = number_of_vertices/number_of_threads;
	int add = 0,i;
	//setting the start and end for each partition
	for(i = 0 ; i < number_of_threads - 1 ; i++)
	{
		parts[i].set(add,add+temp-1);
		add = add+temp;
	}
	parts[i].set(add,number_of_vertices-1);
	return;
}

//function to check if the target is present in specified region in the vector
bool is_present(vector<int> vertices,int target,int start,int end)
{
	//set the flag to false
	bool flag = false;
	//iterate over the region and return the true flag if found
	for(int i = start ; i <= end ; i++)
	{
		if(vertices[i] == target)
		{
			flag = true;
			break;
		}
	}
	return flag;
}

//function to colour the graph((coarse - grained)), as several threads invoke the function
//a pointer to graph object is used
void colour_graph_segment(int id, Graph_colouring* graph)
{
	//getting the start and end of the partition
	int vertex,it,count = 0;
	int start = graph->parts[id].getst();
	int end = graph->parts[id].geten();

	//sets for storing the internal vertices, external vertices
	set<int> internal_vertices;
	set<int> external_vertices;

	//running the loop on the partition boundaries
	//to extract the internal and external vertices
	for(it = start ; it <= end ; it++)
	{
		//extracting the vertex from the randomised vertex
		vertex = graph->vertices[it];

		//exploring the neighbours of the found vertex
		for(int is_pre : graph->graph[vertex])
		{
			if(is_pre)
			{
				//boolean flag to check if the neighbour is present
				bool does_exist = is_present(graph->vertices,count,start,end);

				//if exists
				if(does_exist)
				{
					//add to internal vertex only if it is not in external vertices
					if(external_vertices.find(vertex) == external_vertices.end())
					internal_vertices.insert(vertex);
				}
				//if doesn't exist
				else
				{
					//add to the external vertex and remove if it is present in internal vertices
					external_vertices.insert(vertex);
					if(internal_vertices.find(vertex) != internal_vertices.end())
					internal_vertices.erase(vertex);
				}
			}
			count++;
		}
		count = 0;
	}

	//parallel execution of internal vertices
	//extracting a vertex in internal vertices
	for(int ver : internal_vertices)
	{
		count = 0;
		set<int> assignedcolours;
		int colour = 1;
		//exploring the neighbours of the vertex extracted
		for(int it : graph->graph[ver])
		{
			if(it)
			{
				//if already coloured, add it to the set of assigned colours
				if(colours[count])
				assignedcolours.insert(colours[count]);
			}
			count++;
		}

		//check for the colours in the set and choose a colour
		for(int clr : assignedcolours)
		{
			if(clr == colour)
			colour++;
			else
			break;
		}

		//assign the colour to the vertex and set the colour to default 1.
		if(!graph->status[ver])
		{
			colours[ver] = colour;
			graph->status[ver] = true;
		}

		colour = 1;
	}

	//extracting the external vertex
	for(int ver : external_vertices)
	{
		count = 0;
		set<int> assignedcolours;
		int colour = 1;
		
		//lock the segment such that no other thread alter the data segment
		pthread_mutex_lock(&mutexlock);

		//exploring the neighbours of the vertex extracted
		for(int it : graph->graph[ver])
		{
			if(it)
			{
				//if already coloured, add it to the set
				if(colours[count])
				assignedcolours.insert(colours[count]);
			}
			count++;
		}

		//check for the colours in the set and choose a colour
		for(int clr : assignedcolours)
		{
			if(clr == colour)
			colour++;
			else
			break;
		}

		//assign the colour to the vertex and set the colour to default 1.
		if(!graph->status[ver])
		{
			colours[ver] = colour;
			graph->status[ver] = true;
		}
		colour = 1;

		//unlock the thread such that other vertex gets processed in same thread or in the other thread
		pthread_mutex_unlock(&mutexlock);

	}

	return;
}

//main function
int main()
{
	//opening input file
	ifstream file("input_params.txt");
	//opening output file
	ofstream outfile("coarse-out.txt");

	//if the input file is open and is valid to read
	if(file.is_open())
	{
		//fetch the number of threads and number of vertices
		file>>number_of_threads>>number_of_vertices;
		//assign the threads the memory unit
		worker_threads = new thread[number_of_threads];
		//scanning the files boundaries(no use)
		for(int i = 0 ; i < number_of_vertices ; i++)
		file>>temp;

		//a matrix for holding the graph
		vector<vector<int>> cont;
		vector<int> temporary;
		temporary.resize(number_of_vertices);
		
		//fetching the graph
		for(int i = 0 ; i < number_of_vertices ; i++)
		{
			//fetching the first index of the row
			file>>temp;

			for(int j = 0 ; j < number_of_vertices ; j++)
			{
				file>>temp;
				temporary[j] = temp;
			}
			cont.push_back(temporary);
		}

		//initialsing the lock to use
		pthread_mutex_init(&mutexlock,NULL);

		//create a new graph object and assign the graph using constructor
		Graph_colouring* graph = new Graph_colouring(cont);
		//calling the split method to make partitions
		graph->split();

		//time stamps for storing the start time and end time
		struct timeval start,end;
		//recording the start time stamp
		gettimeofday(&start,NULL);

		//calling the thread function to colour the graph
		for(int th_no = 0 ; th_no < number_of_threads ; th_no++)
		worker_threads[th_no] = thread(colour_graph_segment,th_no,graph);

		//waiting for all the threads to join
		for(int th_no = 0 ; th_no < number_of_threads ; th_no++)
		worker_threads[th_no].join();

		//recording the end time stamp
		gettimeofday(&end,NULL);

		//let the initial number be a negative and in the end we get the actual number of different colours used
		int number_of_colours = -9;
		outfile<<"coarse-grained algorithm:"<<endl<<endl;
		
		//string stream object to store the data to be written in the output file
		string str_data;
		stringstream ss(str_data);
		ss<<"Assigned colours:"<<endl;
		
		//using a for loop, the data is stored in the string stream object
		for(int i = 0 ; i < number_of_vertices ; i++)
		{
			ss<<"vertex-"<<i+1<<" colour-"<<colours[i]<<endl;
			number_of_colours = max(number_of_colours,colours[i]);
		}

		//printing the colours to the out file
		outfile<<"Number of distinct colours used: "<<number_of_colours<<endl<<endl;
		//calculating the time taken for computation and then printing
		outfile<<"timetaken in microseconds= "<<(end.tv_sec-start.tv_sec)*1e6+(end.tv_usec-start.tv_usec)<<"\u03BCs"<<endl;
		outfile<<"timetaken in milliseconds= "<<(end.tv_sec-start.tv_sec)*1e3+(end.tv_usec-start.tv_usec)/1e3<<"milli seconds"<<endl<<endl;
		//printing the stored string to the output file
		outfile<<ss.str();

		//destroying the initialised lock object
		pthread_mutex_destroy(&mutexlock);

		//freeing the graph pointer
		delete graph;
	}
	//if the input file is un-readable, throws the error
	else
	cout<<"Unable to open this file!"<<endl;

	//freeing the threads assigned
	delete[] worker_threads;

	//end of the program
	return 0;
}