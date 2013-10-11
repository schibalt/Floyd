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
#include <stdint.h>
#include <stdlib.h>
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
using std::wstring;

#define NUM_THREADS     5
HANDLE hConsole;

const int MAX_CHARS_PER_LINE = 512;
const int MAX_TOKENS_PER_LINE = 20;
const char* const DELIMITER = " ";
vector<wstring> cities;
int nCities;

//let dist be a |V| × |V| array of minimum distances initialized to ∞ (infinity)
int** dist ;
int** next;

int k;
int rpp;

void PrintDist (int** dist);
void PrintDist (int** dist, int k, int i, int j);
void PrintDistances (int** distances);
void PrintNext (int** next);
string Path (int** dist, int** next, int i, int j);
void *Floyd_Row (void *thread_id);
void Floyd_Row ();
int64_t GetTimeMs64();
vector<string> split (string const & input);
vector<int> path (int source, int destination);

int main (int argc, char* argv[])
{
    //hConsole = GetStdHandle (STD_OUTPUT_HANDLE);
    _setmode (_fileno (stdout), _O_U8TEXT);


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
        fin.open (L"simple.dat");   // open a file
        break;
    default:
        fin.open (L"nqmq.dat");   // open a file
    }//switch

    if (!fin.good())
    {
        wprintf (L"File for graph data couldn't be found or couldn't be open.");
        return 1; // exit if file not found
    }

    // read an entire line into memory
    char buf[MAX_CHARS_PER_LINE];
    fin.getline (buf, MAX_CHARS_PER_LINE);
    istringstream (buf) >> nCities;
    cities.push_back (wstring (buf, buf + strlen (buf)));

    for (int city = nCities; city > 0; city--)
    {
        fin.getline (buf, MAX_CHARS_PER_LINE);
        cities.push_back (wstring (buf, buf + strlen (buf)));
    }

    for (int city = nCities; city > 0; city--)
    {
        wstring svcity = cities[city ];
        for (unsigned i = 0; i < svcity.length(); ++i)
            wprintf (L"%c", svcity.at (i));
        wprintf (L"\n");
    }

    //inst arrays;
    //distance is for initial distances
    int** distances = new int*[nCities + 1];

    //let dist be a |V| × |V| array of minimum distances initialized to ∞ (infinity)
    dist = new int*[nCities + 1];
    next = new int*[nCities + 1];

    for (int i = nCities; i > 0; i--)     //for each edge (u,v)
    {
        distances[i] = new int[nCities + 1];
        dist[i] = new int[nCities + 1];//dist[u][v] ← w(u,v)  // the weight of the edge (u,v)
        next[i] = new int[nCities + 1];

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

        if (buf[0] == '-') break;

        istringstream iss (buf);
        int sourceCity = -1, destCity = -1, distance = INT_MAX;

        for (int subdex = 3; subdex > 0; subdex--)     //iterate thice for each substring
        {
            //(there's actually an additional substring that's either empty or whitespace)
            string sub;
            iss >> sub;
            wstring wsdestcity;
            wstring wssourcecity;

            switch (subdex)
            {
            case 1:
                distance = atoi (sub.c_str());
                wprintf (L"distance: %i\n\n", distance);
                break;
            case 2:
                destCity = atoi (sub.c_str());
                wprintf (L"destination: ");
                wsdestcity = cities[destCity];
                for (unsigned i = 0; i < wsdestcity.length(); ++i)
                    wprintf (L"%c", wsdestcity.at (i));
                wprintf (L"\n");
                break;
            case 3:
                sourceCity = atoi (sub.c_str());
                wprintf (L"source: ");
                wssourcecity = cities[sourceCity];
                for (unsigned i = 0; i < wssourcecity.length(); ++i)
                    wprintf (L"%c", wssourcecity.at (i));
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

    for (int experiments = 1; experiments > 0; experiments--)
    {
        int rc = 0;

        //run loop
        int p[] = {1, 2, 4, 7};
        for (int np = 0; np < 4; np++) //runs the algorithm for
            //particular number of processing elements
        {

            rpp = nCities / p[np]; //rows per processor
            vector<pthread_t> threads (p[np]);
            wprintf (L"────────────────────────────────────────────\n%i cities / %i threads = %i cities per thread\n────────────────────────────────────────────\n",
                     nCities, p[np], rpp);
            uint64_t runstarttime = GetTimeMs64();

            for (k = 1; k <= nCities; k++)
            {
                if (p[np] == 1)
                    Floyd_Row();
                else
                {
                    //wprintf (L"──────\nk = %i\n──────\n", k);
                    //system(L"cls");

                    for (int thrid = 0; thrid < p[np]; thrid++)
                        rc = pthread_create (&threads[thrid], NULL, Floyd_Row, (void *) thrid);

                    for (int thrid = 0; thrid < p[np]; thrid++)
                        rc = pthread_join (threads[thrid], NULL);
                }
            }//k
            uint64_t runendtime = GetTimeMs64();
            uint64_t runtime = runendtime - runstarttime;

            wprintf (L"run time for %i processors: %d ns\n", p[np], runtime);
            //PrintDist (dist);
        }//np
        //run loop end
    }

    PrintNext (next);

    /* initialize random seed: */
    srand ( (unsigned int) time (NULL));
    string buffer;
    /*
        while (buffer != "n" )
        {*/
    int source = rand() % nCities + 1;
    int destination = rand() % nCities + 1;

    while (source == destination)
        destination = rand() % nCities + 1;

    vector<int> vecpath = path (source, destination);
    wprintf (L"\nShortest path from %s to %s is %i.\n", cities[source].c_str(), cities[destination].c_str(), dist[source][destination]);

    for (vector<int>::size_type i = 0; i < vecpath.size() - 1; i++)
    {
        int source = vecpath[i];
        int destination = vecpath[i + 1];
        wprintf (L"    %s to %s:\t%i\n", cities[source].c_str(), cities[destination].c_str(), dist[source][destination]);
    }
    wprintf (L"\nRun again? (y/n)\n", cities[source].c_str(), cities[destination].c_str(), dist[source][destination]);
    /*
    	cin >> buffer;

    while(buffer != "y" && buffer != "n")
    	cin >> buffer;
    }*/

    for (int x = 1; x <= nCities; x++)
    {
        delete [] distances[x];
        delete [] dist[x];
        delete [] next[x];
    }
    delete [] distances;
    delete [] dist;
    delete [] next;
    exit (EXIT_SUCCESS);
}

vector<int> path (int source, int destination)
{
    string wspath = Path (dist, next, source, destination);
    //wprintf (L"\npath: %i%ls%i\n", source, wspath.c_str(), destination);
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

vector<string> split (string const & input)
{
    istringstream buffer (input);
    vector<string> ret;

    copy (istream_iterator<string> (buffer), istream_iterator<string>(), back_inserter (ret));
    return ret;
}

void *Floyd_Row (void * thread_id)
{
    //wprintf (L"Thread %d says, \"Hello!\"\n", thread_id);

    for (int i = (int) thread_id * rpp + 1; i < ( (int) thread_id + 1) * rpp + 1; i++)
    {
        //wprintf (L"processing row %i in thread %d\n", i, thread_id);
        for (int j = 1; j <= nCities; j++)
        {
            int polldist;
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
                next[i][j] = k;
            }
        } //j
    }//i
    pthread_exit (NULL);
    return NULL;
}

void Floyd_Row ()
{
    for (int i = 1; i <= nCities; i++)
    {
        for (int j = 1; j <= nCities; j++)
        {
            //PrintDist (dist, k, i, j);
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
                next[i][j] = k;
            }
        } //j
    }//i
}

string Path (int** dist, int** next, int i, int j)
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
        string pathiint = Path (dist, next, i, intermediate);
        string pathintj = Path (dist, next, intermediate, j);
        stringstream sstm;
        sstm << pathiint << intermediate << pathintj;

        return sstm.str();
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
    wprintf (L"   |");
    for (int x = 1; x <= nCities; x ++)
        wprintf (L" %04i ", x);
    wprintf (L"\n");
    wprintf (L"───┼");
    for (int x = 1; x <= nCities; x ++)
        wprintf (L"──────");
    wprintf (L"\n");
    for (int y = 1; y <= nCities; y ++)
    {
        //wprintf (L"%02i |", y);
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
                //wprintf (L" inf  ");
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
    wprintf (L"\nInitial distances matrix\n\n");
    wprintf (L"   |");
    //wprintf (L"   │");
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
                wprintf (L" inf ");
            //wprintf (L"  \x221e  ");
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
        wprintf (L" %02i ", x);
    wprintf (L"\n");
    wprintf (L"───┼");
    for (int x = 1; x <= nCities; x++)
        wprintf (L"────");
    wprintf (L"\n");
    for (int x = 1; x <= nCities; x++)
    {
        wprintf (L"%02i │", x);
        for (int y = 1; y <= nCities; y++)
        {
            int nextcity = next[x][y];

            if (nextcity == -1)
                wprintf (L" \u00D7  ");
            else
                wprintf (L" %02i ", nextcity);
        }
        wprintf (L"\n");
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
    ret /= 10; // From 100 nano seconds (10^-7)

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