#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>
#include <sstream>

using std::ifstream;
using std::cout;
using std::endl;
using std::string;
using std::size_t;
using std::vector;
using std::istringstream;

#define NUM_THREADS     5

const int MAX_CHARS_PER_LINE = 512;
const int MAX_TOKENS_PER_LINE = 20;
const char* const DELIMITER = " ";
vector<string> cities;
int nCities;

/*
void *TaskCode(void *argument)
{
int tid;

tid = *((int *) argument);
printf("Hello World! It's me, thread %d!\n", tid);

// optionally: insert more useful stuff here

return NULL;
}
*/
int main(void)
{

    // create a file-reading object
    ifstream fin;
    fin.open("nqmq.dat"); // open a file

    if (!fin.good())
        return 1; // exit if file not found

    // read an entire line into memory
    char buf[MAX_CHARS_PER_LINE];
	fin.getline(buf, MAX_CHARS_PER_LINE);
	istringstream (  buf ) >> nCities;

    for(int city = nCities; city > 0; city--) {
        fin.getline(buf, MAX_CHARS_PER_LINE);
        cities.push_back(buf);
    }
	/*
    for (vector<string>::iterator it = cities.begin() ; it != cities.end(); ++it) {
        string city =  *it;
        cout << city.c_str();
    cout << '\n';
    }
	*/
	for(int city=nCities;city>0;city--)
		cout << cities[city-1] << endl;
    cout << '\n';
	cout << "first line is " << nCities << " and there are " << cities.size() << " cities in the vector" << endl;

    /*
    pthread_t threads[NUM_THREADS];
    int thread_args[NUM_THREADS];
    int rc, i;

    // create all threads
    for (i = 0; i < NUM_THREADS; ++i) {
    thread_args[i] = i;
    printf("In main: creating thread %d\n", i);
    rc = pthread_create(&threads[i], NULL, TaskCode, (void *) &thread_args[i]);
    assert(0 == rc);
    }

    // wait for all threads to complete
    for (i = 0; i < NUM_THREADS; ++i) {
    rc = pthread_join(threads[i], NULL);
    assert(0 == rc);
    }
    */

    exit(EXIT_SUCCESS);
}