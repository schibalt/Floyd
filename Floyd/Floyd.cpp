#include <assert.h>
#include <cmath>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <pthread.h>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <ctime>
#endif

using std::cin;
using std::cout;
using std::distance;
using std::endl;
using std::find;
using std::ifstream;
using std::istream_iterator;
using std::istringstream;
using std::locale;
using std::pow;
using std::setw;
using std::setfill;
using std::size_t;
using std::string;
using std::stringstream;
using std::transform;
using std::vector;

//HANDLE hConsole;

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
uint64_t GetTimeMs64();
vector<string> split (string const & ilooput);
vector<int> GetPath (int source, int destination);
void InitializeArrays();
void CleanUpArrays();
void PathLoop();
bool CompareStrings (string citytofind, string cityinvector);
int FindCity();

// A shared mutex
pthread_mutex_t mutex;

int main (int argc, char* argv[])
{
    //hConsole = GetStdHandle (STD_OUTPUT_HANDLE);
    //_setmode (_fileno (stdout), _O_U8TEXT);

    filename = argv[1];
    if (atoi (argv[2]))
        bidirectional = 1;
    else
        bidirectional = 0;

    processors = atoi (argv[3]);

    InitializeArrays();
    rpp = nCities / processors; //rows per processor
    if (rpp < 1) // having more threads than rows isn't necessary
    {
        printf ("using more processors than there are cities doesn't make sense\n");
        exit (0);
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
        int thread;

        for (thread = 0; thread < processors; thread++)
        {
            //printf ("creating thread %d for k %02d\n", thread, k);
            if (pthread_create (&threads[thread], NULL, Floyd_Row, (void*) (uintptr_t) (int) thread))
            {
                printf ("Could not create thread %d\n", thread);
                exit (0);
            }
        }

        for (thread = 0; thread < processors; thread++)
        {
            //printf ("joining thread %d for k %02d\n", thread, k);
            if (pthread_join (threads[thread], NULL))
            {
                printf ("Could not join thread %d\n", thread);
                exit (0);
            }
            //printf ("Main: completed join with thread %ld having a status of %ld\n", thread, (long) status);
        }

        //printf ("\nrun time for k = %02d with %i processors: %d ms", k, processors, runtime);
        //printf ("\n%d", runtime);
    }//k

    uint64_t runendtime = GetTimeMs64();
    uint64_t runtime = runendtime - runstarttime;

    printf ("\nrun time for %i processors: %lu us\n", processors, runtime);

    //PrintDist (dist);
    //PrintNext (next);
    PathLoop();
    CleanUpArrays();

    pthread_exit (NULL);
    exit (EXIT_SUCCESS);
}

uint64_t GetTimeMs64()
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
    ret /= 10; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */

    return ret;
#else
    /* Linux */
    struct timeval tv;

    gettimeofday (&tv, NULL);

    uint64_t ret = tv.tv_usec;
    /* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
//ret /= 1000;

    /* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
    ret += (tv.tv_sec * 1000);

    return ret;
#endif
}

void *Floyd_Row (void * thread_id)
{
    // Lock the mutex
    //pthread_mutex_lock(&mutex);
    //printf ("Thread %d says, \"Hello!\"\n", thread_id);
    int  threadid = * ( (int*) (&thread_id));

    for (int i = threadid * rpp + 1; i <= (threadid + 1) * rpp; i++)
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

vector<int> GetPath (int source, int destination)
{
    string spath = Path (source, destination);
    //printf ("\loopath: %i%ls%i\n", source, wspath.c_str(), destination);
    //const string spath (spath.begin(), spath.end());
    vector<int> pathvector;

    pathvector.push_back (source);
    vector<string> intermediatepath = split (spath);

    for (vector<string>::iterator it = intermediatepath.begin(); it != intermediatepath.end(); ++it)
    {
        int city = atoi ( (*it).c_str());
        pathvector.push_back (city);
    }

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
//
//void PrintDist (int** dist)
//{
//    //print dist matrix
//    printf ("\n");
//    printf ("Shortest paths matrix\n\n");
//    printf ("   │");
//    for (int x = 1; x <= nCities; x ++)
//        printf (" %04i ", x);
//    printf ("\n");
//    printf ("───┼");
//    for (int x = nCities; x > 0; x--)
//        printf ("──────");
//    printf ("\n");
//    for (int x = 1; x <= nCities; x ++)
//    {
//        printf ("%02i │", x);
//        for (int y = 1; y <= nCities; y ++)
//        {
//            int distance = dist[x][y];
//
//            if (distance == INT_MAX)
//                printf (" inf  ");
//            //printf ("  \x221e   ");
//            else
//                printf (" %04i ", distance);
//        }
//        printf ("\n");
//    }
//    printf ("\n");
//}
//
//void PrintDist (int** dist, int k, int i, int j)
//{
//    //system("cls");
//    printf ("\n");
//    printf ("Shortest paths matrix (k = %i, i = %i, j = %i)\n\n", k, i, j);
//    printf ("   |");
//    for (int x = 1; x <= nCities; x ++)
//        printf (" %04i ", x);
//    printf ("\n");
//    printf ("───┼");
//    for (int x = 1; x <= nCities; x ++)
//        printf ("──────");
//    printf ("\n");
//    for (int y = 1; y <= nCities; y ++)
//    {
//        printf ("%02i │", y);
//        for (int x = 1; x <= nCities; x ++)
//        {
//            int distance = dist[y][x];
//
//            //color logic
//            bool isoffset = y == i && x == k || y == k && x == j;
//            bool isfocal = y == i && x == j;
//
//            if (isoffset && !isfocal)
//                SetConsoleTextAttribute (hConsole, 14);
//
//            if (!isoffset && isfocal)
//                SetConsoleTextAttribute (hConsole, 12);
//
//            if (isoffset && isfocal)
//                SetConsoleTextAttribute (hConsole, 4);
//            //end color logic
//
//            if (distance == INT_MAX)
//                printf (" inf  ");
//            //printf ("  \x221e   ");
//            else
//                printf (" %04i ", distance);
//            SetConsoleTextAttribute (hConsole, 15);
//        }
//        printf ("\n");
//    }
//    printf ("\n");
//}
//
//void PrintNext (int** next)
//{
//    //print distances matrix
//    printf ("Path matrix\n\n");
//    printf ("   │");
//    for (int x = 1; x <= nCities; x++)
//        printf (" %02i ", x);
//    printf ("\n");
//    printf ("───┼");
//    for (int x = 1; x <= nCities; x++)
//        printf ("────");
//    printf ("\n");
//    for (int x = 1; x <= nCities; x++)
//    {
//        printf ("%02i │", x);
//        for (int y = 1; y <= nCities; y++)
//        {
//            int nextcity = next[x][y];
//
//            if (nextcity == -1)
//                printf (" \u00D7  ");
//            else
//                printf (" %02i ", nextcity);
//        }
//        printf ("\n");
//    }
//}

void InitializeArrays()
{
    fin.open (filename.c_str());   // open a file

    if (!fin.good())
    {
        printf ("File for graph data couldn't be found or couldn't be open.");
        exit (0); // exit if file not found
    }

    fin.getline (buf, MAX_CHARS_PER_LINE);
    istringstream (buf) >> nCities;
    cities.push_back (string (buf, buf + strlen (buf)));

    for (int cityidx = nCities; cityidx > 0; cityidx--)
    {
        fin.getline (buf, MAX_CHARS_PER_LINE);
        string city = string (buf, buf + strlen (buf));
        transform (city.begin(), city.end(), city.begin(), ::tolower);
        cities.push_back (city);
        //string svcity = cities[nCities - city + 1];
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

//doesn't work on UNIX
int FindCityWindows()
{
    cin >> buf;
    string city = string (buf, buf + strlen (buf));
    transform (city.begin(), city.end(), city.begin(), ::tolower);

    vector<string>::iterator iter = find (cities.begin(), cities.end(), city);
    size_t index = distance (cities.begin(), iter);
    if (index == cities.size())
    {
        printf ("\nThere isn't any information available about %s.", city.c_str());
        printf ("\nEnter the name of the city again.\n");
        return FindCity();
    }
    else
    {
        //printf ("\n%s is city %d.\n", city.c_str(), index);
        return index;
    }
}

int FindCity()
{
    cin >> buf;
    int city = atoi (buf);

    if (city < 1 || city > nCities)
    {
        printf ("\nEnter a valid id.\n");
        return FindCity();
    }
    else
    {
        printf ("\n%s is city %d.\n", cities[city].c_str(), city);
        return city;
    }
}

void PathLoop()
{
    string buffer;

    while (buffer[0] != 'n')
    {
        printf ("\n");
        for (int city = nCities; city > 0; city--)
            printf ("%02i  %s\n", nCities - city + 1, cities[nCities - city + 1].c_str());

        int source = 0;
        int destination = 0;
        while (source == destination)
        {
            printf ("\nEnter the id of the origin city.\n");

            source = FindCity();

            printf ("\nEnter the id of the destination city.\n");

            destination = FindCity();
        }

        vector<int> pathvector = GetPath (source, destination);	//then get intermediate path

        printf ("\nShortest path from %s to %s is %d.\n", cities[source].c_str(), cities[destination].c_str(), dist[source][destination]);

        for (unsigned int i = 0; i < pathvector.size() - 1; i++)
        {
            int from = pathvector[i];
            int to = pathvector[i + 1];
            printf ("    %s to %s:\t%i\n", cities[from].c_str(), cities[to].c_str(), dist[from][to]);
            //cout << "    " << cities[from] << " to " << cities[to] << ":\t" << dist[from][to] << "\n";
        }
        printf ("\nRun again? (y/n)\n");

        cin >> buffer;

        while (buffer[0] != 'y' && buffer[0] != 'n')
            cin >> buffer;
    } //test loop
}