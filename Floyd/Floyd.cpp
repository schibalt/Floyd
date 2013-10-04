#include <assert.h>
#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <io.h>
#include <iomanip>
#include <pthread.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <vector>

using std::cout;
using std::endl;
using std::ifstream;
using std::istringstream;
using std::pow;
using std::setw;
using std::setfill;
using std::size_t;
using std::string;
using std::vector;
using std::to_string;

#define NUM_THREADS     5
HANDLE hConsole;

const int MAX_CHARS_PER_LINE = 512;
const int MAX_TOKENS_PER_LINE = 20;
const char* const DELIMITER = " ";
vector<string> cities;
int nCities;

void PrintDist (int** dist);
void PrintDist (int** dist, int k, int i, int j);
void PrintDistances (int** distances);
void PrintNext (int** next);
string Path (int** dist, int** next, int i, int j);

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
int main (int argc, char* argv[])
{
    hConsole = GetStdHandle (STD_OUTPUT_HANDLE);
    _setmode (_fileno (stdout), _O_U16TEXT);

    int testcase = atoi (argv[1]);
    bool bidirectional;
    if (atoi (argv[2]) == 1)
        bidirectional = 1;
    else
        bidirectional = 0;

    // create a file-reading object
    ifstream fin;

    switch (testcase)     //decide which case is going to be run
    {
    case 1:
        fin.open ("simple.dat");   // open a file
        break;
    default:
        fin.open ("nqmq.dat");   // open a file
    }//switch

    if (!fin.good())
        return 1; // exit if file not found

    // read an entire line into memory
    char buf[MAX_CHARS_PER_LINE];
    fin.getline (buf, MAX_CHARS_PER_LINE);
    istringstream (buf) >> nCities;
    cities.push_back (buf);

    for (int city = nCities; city > 0; city--)
    {
        fin.getline (buf, MAX_CHARS_PER_LINE);
        cities.push_back (buf);
    }
    /*
    for (vector<string>::iterator it = cities.begin() ; it != cities.end(); ++it) {
        string city =  *it;
        cout << city.c_str();
    cout << '\n';
    }
    */
    for (int city = nCities; city > 0; city--)
    {
        string svcity = cities[city ];
        for (unsigned i = 0; i < svcity.length(); ++i)
            wprintf (L"%c", svcity.at (i));
        wprintf (L"\n");
    }

    //inst arrays;
    //distance is for initial distances
    int** distances = new int*[nCities + 1];

    //let dist be a |V| × |V| array of minimum distances initialized to ∞ (infinity)
    int** dist = new int*[nCities + 1];
    int** next = new int*[nCities + 1];

    for (int i = nCities; i > 0; i--)     //for each edge (u,v)
    {
        distances[i] = new int[nCities];
        dist[i] = new int[nCities];//dist[u][v] ← w(u,v)  // the weight of the edge (u,v)
        next[i] = new int[nCities];

        for (int j = nCities; j > 0; j--)
        {
            if (i == j)
            {
                distances[i][j] = 0;
                dist[i][j] = 0;
            }
            else
            {
                distances[i][j] = INT_MAX;
                dist[i][j] = INT_MAX;
            }
            next[i][j] = -1;
        }
    }
    //arrays initialized

    //parse initial city distances
    while (!fin.eof())
    {
        fin.getline (buf, MAX_CHARS_PER_LINE);

        if (buf[0] == '-')
            break;

        istringstream iss (buf);
        int sourceCity = -1, destCity = -1, distance = INT_MAX;

        for (int subdex = 3; subdex > 0; subdex--)     //iterate thice for each substring
        {
            //(there's actually an additional substring that's either empty or whitespace)
            string sub;
            iss >> sub;
            string svdestcity;
            string svsourcecity;

            switch (subdex)
            {
            case 1:
                distance = atoi (sub.c_str());
                wprintf (L"distance: %i\n", distance);
                break;
            case 2:
                destCity = atoi (sub.c_str());
                wprintf (L"destination: ");
                svdestcity = cities[destCity];
                for (unsigned i = 0; i < svdestcity.length(); ++i)
                    wprintf (L"%c", svdestcity.at (i));
                wprintf (L"\n");
                break;
            case 3:
                sourceCity = atoi (sub.c_str());
                wprintf (L"source: ");
                svsourcecity = cities[sourceCity  ];
                for (unsigned i = 0; i < svsourcecity.length(); ++i)
                    wprintf (L"%c", svsourcecity.at (i));
                wprintf (L"\n");
                break;
            default:
                wprintf (L"this isn't the substring index you're looking for\n");
            }   //switch
        }//for

        distances[sourceCity][destCity  ] = distance;

        if (bidirectional)
            distances[destCity ][sourceCity ] = distance;

        if (distance != INT_MAX)     //inf dist edges already initialized
        {
            //only overwrite edge vals is they're explicitly finite
            //(safety from overwriting zero-val edges from each vertex to itself)
            dist[sourceCity ][destCity ] = distance;

            if (bidirectional)
                dist[destCity ][sourceCity ] = distance;
        }
    } //while
    //initial distances initialized

    PrintDistances (distances);
    PrintDist (dist);
    PrintNext (next);

    for (int k = 1; k <= nCities; k++)
    {
        //system("cls");
        for (int i = 1; i <= nCities; i++)
        {
            for (int j = 1; j <= nCities; j++)
            {
				if(j==nCities && i%2==0)
                PrintDist (dist, k, i, j);
                int polldist ;
                int offsetbycolumn = dist[i][k];
                int offsetbyrow = dist[k][j];

                //the sum of a negative weight edge and an infinite weight edge only
                //seems less than an infinite weight edge because of the use of int data type.
                //in principle the sum is still infinite.
                if (offsetbyrow == INT_MAX || offsetbycolumn == INT_MAX)
                    polldist = INT_MAX;
                else
                    polldist = dist[i][k] + dist[k][j];

                if (polldist < dist[i][j])
                {
                    dist[i][j] = polldist;
                    next[i][j] = k ;
                }
            } //j
        }//i
    }//k

    PrintNext (next);
    string path = Path (dist, next, 4, 3);

    for (int x = 0; x <= nCities; x++)
    {
        delete [] distances[x] ;
        delete [] dist[x] ;
        delete [] next[x] ;
    }
    delete [] distances;
    delete [] dist;
    delete [] next;
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

    exit (EXIT_SUCCESS);
}

string Path (int** dist, int** next, int i, int j)
{
    if (dist[i][j] == INT_MAX)
        return "no path";//no path

    string intermediate = " ";
    intermediate = to_string (next[i][j]);
    if (intermediate == "-1")
        return " ";   // the direct edge from i to j gives the shortest path
    else
    {
        string pathiint = Path (dist, next, i, atoi (intermediate.c_str()));
        string pathintj = Path (dist, next, atoi (intermediate.c_str()), j);

        return pathiint + intermediate + pathintj;
    }
}

void PrintDist (int** dist)
{
    //print dist matrix
    wprintf (L"\n");
    wprintf (L"Shortest paths matrix\n\n");
    wprintf (L"   │");
    for (int x = 1; x <= nCities; x ++)
        wprintf (L" %04i ", x);
    wprintf (L"\n");
    wprintf (L"───┼");
    for (int x = nCities; x > 0; x--)
        wprintf (L"──────");
    wprintf (L"\n");
    for (int x = 1; x <= nCities; x ++)
    {
        wprintf (L"%02i │", x);
        for (int y = 1; y <= nCities; y ++)
        {
            int distance = dist[x][y];

            if (distance == INT_MAX)
                wprintf (L"  \x221e   ");
            else
                wprintf (L" %04i ", distance);
        }
        wprintf (L"\n");
    }
    wprintf (L"\n");
}

void PrintDist (int** dist, int k, int i, int j)
{
    //print dist matrix
    wprintf (L"\n");
    wprintf (L"Shortest paths matrix (k = %i, i = %i, j = %i)\n\n", k, i, j);
    wprintf (L"   │");
    for (int x = 1; x <= nCities; x ++)
        wprintf (L" %04i ", x);
    wprintf (L"\n");
    wprintf (L"───┼");
    for (int x = 1; x <= nCities; x ++)
        wprintf (L"──────");
    wprintf (L"\n");
    for (int y = 1; y <= nCities; y ++)
    {
        wprintf (L"%02i │", y);
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

            if (distance == INT_MAX)
                wprintf (L"  \x221e   ");
            else
                wprintf (L" %04i ", distance);
            SetConsoleTextAttribute (hConsole, 15);
        }
        wprintf (L"\n");
    }
    wprintf (L"\n");
}

void PrintDistances (int** distances)
{
    //print distances matrix
    wprintf (L"\n");
    wprintf (L"Initial distances matrix\n\n");
    wprintf (L"   │");
    for (int x = 1; x <= nCities; x ++)
        wprintf (L" %03i ", x);
    wprintf (L"\n");
    wprintf (L"───┼");
    for (int x = 1; x <= nCities; x ++)
        wprintf (L"─────");
    wprintf (L"\n");
    for (int x = 1; x <= nCities; x ++)
    {
        wprintf (L"%02i │", x);
        for (int y = 1; y <= nCities; y ++)
        {
            int distance = distances[x][y];

            if (distance == INT_MAX)
                wprintf (L"  \x221e  ");
            else
                wprintf (L" %03i ", distance);
        }
        wprintf (L"\n");
    }
    wprintf (L"\n");
}

void PrintNext (int** next)
{
    //print distances matrix
    wprintf (L"Path matrix\n\n");
    wprintf (L"   │");
    for (int x = 1; x <= nCities; x++)
        wprintf (L" %03i ", x);
    wprintf (L"\n");
    wprintf (L"───┼");
    for (int x = 1; x <= nCities; x++)
        wprintf (L"─────");
    wprintf (L"\n");
    for (int x = 1; x <= nCities; x++)
    {
        wprintf (L"%02i │", x);
        for (int y = 1; y <= nCities; y++)
        {
            int nextcity = next[x][y];

            if (nextcity == INT_MAX)
                wprintf (L"  \x221e  ");
            else
                wprintf (L" %03i ", nextcity);
        }
        wprintf (L"\n");
    }
}