//#include <assert.h>
//#include <cmath>
//#include <cstring>
//#include <fcntl.h>
#include <fstream>
#include <iostream>
//#include "io.h"
#include <iomanip>
#include <pthread.h>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
//#include <stdlib.h>
#include <vector>
#include <Windows.h>

using std::cout;
using std::endl;
using std::ifstream;
using std::istringstream;
using std::pow;
using std::setw;
using std::setfill;
using std::size_t;
using std::string;
using std::to_string;
using std::vector;
using std::locale;
using std::string;
using std::stringstream;
using std::istream_iterator;
using std::cin;

HANDLE hConsole;

const int MAX_CHARS_PER_LINE = 512;
const int MAX_TOKENS_PER_LINE = 20;
// read an entire line into memory
char buf[MAX_CHARS_PER_LINE];
const char* const DELIMITER = " ";
vector<string> cities;
int nCities;

//let dist be a |V| × |V| array of minimum distances initialized to ∞ (infinity)'
int** dist ;
int** next;

int k;
int rpp;
bool bidirectional;
string filename;
int processors;

// create a file-reading object
ifstream fin;

void PrintDist (int** dist);
void PrintDist (int** dist, int k, int i, int j);
void PrintDistances (int** distances);
void PrintNext (int** next);
string Path (int i, int j);
void *Floyd_Row (void *thread_id);
int64_t GetTimeMs64();
vector<string> split (string const & ilooput);
vector<int> GetPath (int source, int destination);
void InitializeArrays();
void CleanUpArrays();
void PathLoop();

// A shared mutex
pthread_mutex_t mutex;

int main (int argc, char* argv[])
{
	hConsole = GetStdHandle (STD_OUTPUT_HANDLE);
	//_setmode (_fileno (stdout), _O_U8TEXT);

	filename = argv[1];
	if (atoi (argv[2]) )
		bidirectional = 1;
	else
		bidirectional = 0;

	processors = atoi(argv[3]);

	/* initialize random seed: */
	srand ( (unsigned int) time (NULL));

	int rc = 0;

	InitializeArrays();
	rpp = nCities / processors; //rows per processor
	if (rpp < 1) // having more threads than rows isn't necessary
	{
		printf ("using more processors than there are cities doesn't make sense\n");
		exit(0);
	}
	/*
	PrintDistances (distances);
	PrintDist (dist);
	PrintNext (next);*/
	vector<pthread_t> threads (processors);

	printf ("────────────────────────────────────────────\n%i cities / %i threads = %i cities per thread\n────────────────────────────────────────────\n",
		nCities, processors, rpp);
	
	uint64_t runstarttime = GetTimeMs64();
	for (k = 1; k <= nCities; k++)
	{
			//printf ("──────\nk = %i\n──────\n", k);

			// Initialize the mutex
			/*
			if(pthread_mutex_init(&mutex, NULL))
			{
			printf("Unable to initialize a mutex\n");
			return -1;
			}*/

			for (int thread = 0; thread < processors; thread++)
			{
				if(pthread_create (&threads[thread], NULL, Floyd_Row, (void *) thread))
				{
					printf("Could not create thread %d\n", thread);
					exit(0);
				}
			}

			for (int thread = 0; thread < processors; thread++)
			{
				if(pthread_join (threads[thread], NULL))
				{
					printf("Could not join thread %d\n", thread);
					exit(0);
				}
			}
	}//k
	uint64_t runendtime = GetTimeMs64();
	uint64_t runtime = runendtime - runstarttime;

	printf ("\nrun time for %i processors: %d ms\n", processors, runtime);
	//PrintDist (dist);
	//PrintNext (next);
	PathLoop();
	CleanUpArrays();

	exit (EXIT_SUCCESS);
}

vector<int> GetPath (int source, int destination)
{
	string wspath = Path (source, destination);
	//printf ("\loopath: %i%ls%i\n", source, wspath.c_str(), destination);
	const string spath (wspath.begin(), wspath.end());
	vector<int> pathvector;
	char buffer[3];

	string scity (buffer);

	pathvector.push_back (source);
	vector<string> intermediatepath = split (spath);

	for (vector<string>::iterator it = intermediatepath.begin(); it != intermediatepath.end(); ++it)
	{
		int city = atoi ( (*it).c_str());
		pathvector.push_back (city);
	}

	_itoa_s (destination, buffer, 10);
	scity = string (buffer);
	pathvector.push_back (destination);

	return pathvector;
}

vector<string> split (string const & ilooput)
{
	istringstream buffer (ilooput);
	vector<string> ret;

	copy (istream_iterator<string> (buffer), istream_iterator<string>(), back_inserter (ret));
	return ret;
}

void *Floyd_Row (void * thread_id)
{
	// Lock the mutex
	//pthread_mutex_lock(&mutex);

	//printf ("Thread %d says, \"Hello!\"\n", thread_id);

	for (int i = (int) thread_id * rpp + 1; i <= ( (int) thread_id + 1) * rpp; i++)
	{
		//printf ("processing row %02i in thread %d\n", i, thread_id);
		for (int j = 1; j <= nCities; j++)
		{
			//PrintDist (dist, k, i, j);
			int polldist;
			int kthcolumn = dist[i][k];
			int kthrow = dist[k][j];

			//the sum of a negative weight edge and an infinite weight edge only
			//seems less than an infinite weight edge because of the use of int data type.
			//in principle the sum is still infinite.
			if (kthrow == INT_MAX || kthcolumn == INT_MAX)
				polldist = INT_MAX;
			else
				polldist = dist[i][k] + dist[k][j];

			if (polldist < dist[i][j])
			{
				dist[i][j] = polldist;
				next[i][j] = k;
			}
		} //j
	}//i
	// Unlock the mutex
	//pthread_mutex_unlock(&mutex);

	pthread_exit (NULL);
	return NULL;
}

string Path (int i, int j)
{
	if (dist[i][j] == INT_MAX)
		return "no path";//no path

	string sintermediate = " ";
	sintermediate = next[i][j];
	int intermediate = next[i][j];

	if (intermediate == -1)
		return " ";   // the direct edge from i to j gives the shortest path
	else
	{
		string pathiint = Path (i, intermediate);
		string pathintj = Path (intermediate, j);
		stringstream sstm;
		sstm << pathiint << intermediate << pathintj;

		return sstm.str();
	}
}

void PrintDist (int** dist)
{
	//print dist matrix
	printf ("\n");
	printf ("Shortest paths matrix\n\n");
	printf ("   │");
	for (int x = 1; x <= nCities; x ++)
		printf (" %04i ", x);
	printf ("\n");
	printf ("───┼");
	for (int x = nCities; x > 0; x--)
		printf ("──────");
	printf ("\n");
	for (int x = 1; x <= nCities; x ++)
	{
		printf ("%02i │", x);
		for (int y = 1; y <= nCities; y ++)
		{
			int distance = dist[x][y];

			if (distance == INT_MAX)
				printf (" inf  ");
				//printf ("  \x221e   ");
			else
				printf (" %04i ", distance);
		}
		printf ("\n");
	}
	printf ("\n");
}

void PrintDist (int** dist, int k, int i, int j)
{
	//system("cls");
	printf ("\n");
	printf ("Shortest paths matrix (k = %i, i = %i, j = %i)\n\n", k, i, j);
	printf ("   |");
	for (int x = 1; x <= nCities; x ++)
		printf (" %04i ", x);
	printf ("\n");
	printf ("───┼");
	for (int x = 1; x <= nCities; x ++)
		printf ("──────");
	printf ("\n");
	for (int y = 1; y <= nCities; y ++)
	{
		printf ("%02i │", y);
		for (int x = 1; x <= nCities; x ++)
		{
			int distance = dist[y][x];

			//color logic
			bool isoffset = y == i && x == k || y == k && x == j;
			bool isfocal = y == i && x == j;

			if (isoffset && !isfocal)
				SetConsoleTextAttribute (hConsole, 14);

			if (!isoffset && isfocal)
				SetConsoleTextAttribute (hConsole, 12);

			if (isoffset && isfocal)
				SetConsoleTextAttribute (hConsole, 4);
			//end color logic

			if (distance == INT_MAX)
				printf (" inf  ");
				//printf ("  \x221e   ");
			else
				printf (" %04i ", distance);
			SetConsoleTextAttribute (hConsole, 15);
		}
		printf ("\n");
	}
	printf ("\n");
}

void PrintDistances (int** distances)
{
	//print distances matrix
	printf ("\nInitial distances matrix\n\n");
	printf ("   |");
	//printf ("   │");
	for (int x = 1; x <= nCities; x ++)
		printf (" %03i ", x);
	printf ("\n");
	printf ("───┼");
	for (int x = 1; x <= nCities; x ++)
		printf ("─────");
	printf ("\n");
	for (int x = 1; x <= nCities; x ++)
	{
		printf ("%02i │", x);
		for (int y = 1; y <= nCities; y ++)
		{
			int distance = distances[x][y];

			if (distance == INT_MAX)
				//printf ("  \x221e  ");
				printf (" inf ");
			else
				printf (" %03i ", distance);
		}
		printf ("\n");
	}
	printf ("\n");
}

void PrintNext (int** next)
{
	//print distances matrix
	printf ("Path matrix\n\n");
	printf ("   │");
	for (int x = 1; x <= nCities; x++)
		printf (" %02i ", x);
	printf ("\n");
	printf ("───┼");
	for (int x = 1; x <= nCities; x++)
		printf ("────");
	printf ("\n");
	for (int x = 1; x <= nCities; x++)
	{
		printf ("%02i │", x);
		for (int y = 1; y <= nCities; y++)
		{
			int nextcity = next[x][y];

			if (nextcity == -1)
				printf (" \u00D7  ");
			else
				printf (" %02i ", nextcity);
		}
		printf ("\n");
	}
}

int64_t GetTimeMs64()
{
#ifdef WIN32
	/* Windows */
	FILETIME ft;
	LARGE_INTEGER li;

	/* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
	* to a LARGE_INTEGER structure. */
	GetSystemTimeAsFileTime (&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	uint64_t ret = li.QuadPart;
	ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
	ret /= 10000; // From 100 nano seconds (10^-7)

	return ret;
#else
	/* Linux */
	struct timeval tv;

	gettimeofday (&tv, NULL);

	uint64 ret = tv.tv_usec;
	/* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
	ret /= 1000;

	/* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
	ret += (tv.tv_sec * 1000);

	return ret;
#endif
}

void InitializeArrays()
{
	fin.open (filename);   // open a file

	if (!fin.good())
	{
		printf ("File for graph data couldn't be found or couldn't be open.");
		exit(0); // exit if file not found
	}

	fin.getline (buf, MAX_CHARS_PER_LINE);
	istringstream (buf) >> nCities;
	cities.push_back (string (buf, buf + strlen (buf)));

	for (int city = nCities; city > 0; city--)
	{
		fin.getline (buf, MAX_CHARS_PER_LINE);
		cities.push_back (string (buf, buf + strlen (buf)));
		string svcity = cities[nCities - city + 1];
		//for (unsigned i = 0; i < svcity.length(); ++i)
		//printf ("%c\n", svcity.at (i));
	}
	//inst arrays;

	//let dist be a |V| × |V| array of minimum distances initialized to ∞ (infinity)
	dist = new int*[nCities + 1];
	next = new int*[nCities + 1];

	for (int i = nCities; i > 0; i--)     //for each edge (u,v)
	{
		dist[i] = new int[nCities + 1];//dist[u][v] ← w(u,v)  // the weight of the edge (u,v)
		next[i] = new int[nCities + 1];

		for (int j = nCities; j > 0; j--)
		{
			if (i == j)
				dist[i][j] = 0;
			else
				dist[i][j] = INT_MAX;
			next[i][j] = -1;
		}
	}
	//arrays initialized

	//parse initial city distances
	while (!fin.eof())
	{
		fin.getline (buf, MAX_CHARS_PER_LINE);

		if (buf[0] == '-') break;

		istringstream iss (buf);
		int sourceCity = -1, destCity = -1, distance = INT_MAX;

		for (int subdex = 3; subdex > 0; subdex--)     //iterate thice for each substring
		{
			//(there's actually an additional substring that's either empty or whitespace)
			string sub;
			iss >> sub;
			string wsdestcity;
			string wssourcecity;

			switch (subdex)
			{
			case 1:
				distance = atoi (sub.c_str());
				//printf ("distance: %i\n", distance);
				break;
			case 2:
				destCity = atoi (sub.c_str());
				//printf ("destination: ");
				wsdestcity = cities[destCity];
				//for (unsigned i = 0; i < wsdestcity.length(); ++i)
				//printf ("%c\n", wsdestcity.at (i));
				break;
			case 3:
				sourceCity = atoi (sub.c_str());
				//printf ("\nsource: ");
				wssourcecity = cities[sourceCity];
				//for (unsigned i = 0; i < wssourcecity.length(); ++i)
				//printf ("%c\n", wssourcecity.at (i));
				break;
			default:
				printf ("this isn't the substring index you're looking for\n");
			}   //switch
		}//for

		if (distance != INT_MAX)     //inf dist edges already initialized
		{
			//only overwrite edge vals is they're explicitly finite
			//(safety from overwriting zero-val edges from each vertex to itself)
			dist[sourceCity][destCity] = distance;

			if (bidirectional)
				dist[destCity][sourceCity] = distance;
		}
	} //while
	fin.close();
	//initial distances initialized
}

void CleanUpArrays()
{
	for (int x = 1; x <= nCities; x++)
	{
		delete [] dist[x];
		delete [] next[x];
	}
	delete [] dist;
	delete [] next;
}

void PathLoop(){
	string buffer;
	/*
	while (buffer != "n" )
	{*/
	//test loop

	const int tests = 5;
	int ** testtrips = new int *[tests];

	for(int y = tests; y > 0; y--)
	{
		testtrips[y - 1] = new int[2];
		/*for(int x = 2; x > 0; x--)
		{
		testtrips[y - 1][x - 1] = rand() % nCities + 1;
		printf ("%i ", testtrips[y - 1][x - 1]);
		}
		printf ("\n ");*/
	}
	testtrips[4][0] = 20; //predetermined test paths
	testtrips[4][1] = 1;
	testtrips[3][0] = 11;
	testtrips[3][1] = 8;
	testtrips[2][0] = 22;
	testtrips[2][1] = 5;
	testtrips[1][0] = 11;
	testtrips[1][1] = 26;
	testtrips[0][0] = 19;
	testtrips[0][1] = 24;

	for(int test = tests; test > 0; test--)
	{
		int source = testtrips[test - 1][0];
		int destination = testtrips[test - 1][1];
		//int source = rand() % nCities + 1;
		//int destination = rand() % nCities + 1;

		while (source == destination)
			destination = rand() % nCities + 1;

		vector<int> pathvector = GetPath (source, destination);	//then get intermediate path

		printf ("\nShortest path from %s to %s is %i.\n", cities[source].c_str(), cities[destination].c_str(), dist[source][destination]);

		for (vector<int>::size_type i = 0; i < pathvector.size() - 1; i++)
		{
			int from = pathvector[i];
			int to = pathvector[i + 1];
			printf ("    %s to %s:\t%i\n", cities[from].c_str(), cities[to].c_str(), dist[from][to]);
		}
		printf ("\nRun again? (y/n)\n");
	} //test loop
	/*
	cin >> buffer;

	while(buffer != "y" && buffer != "n")
	cin >> buffer;
	}*/
}