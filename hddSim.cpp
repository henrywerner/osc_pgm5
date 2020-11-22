/*
file: hddSim.cpp
author: Gherkin
modification history: 
    Gherkin
    November 22nd, 2020
procedures:
    [general methods]
    main - Manages the calls for the testing and printing methods.
    executeAlg - Manages the testing conditions and execution of the scheduling algorithms.
    generateRequests - Randomly generate a collection of simulated I/O requests.

    [disk scheduling algorithms]
    fifo - Emulates the performance of a First-In-First-Out disk scheduling algorithm.
    sstf - Emulates the performance of a Shortest Service Time First disk scheduling algorithm.
    scan - Emulates the performance of a SCAN disk scheduling algorithm.
    lifo - Emulates the performance of a Last-In-First-Out disk scheduling algorithm.

    [print management]
    printResults - Apply stylization and print algorithm test results to the console.
    updateProgressBar - Print a graphical indicator of the current test's progression.

    [helper methods]
    updateSector - Produce an updated sector value based on the given time value.
    quicksort - Use a Quicksort algorithm to sort I/O requests by either Sector or Track values.
    partitionBySector - Quicksort partitioning through comparing Sector values of each I/O request.
    partitionByTrack - Quicksort partitioning through comparing Track values of each I/O request.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <cmath>
#include <chrono>
#include <algorithm>
#include "ioReq.cpp"

using namespace std;

/* Define HDD constants */
#define EXPERIMENTS 1000
#define AVG_SEEK_TIME 0.024875  // Time needed to move the head between tracks in milliseconds
#define AVG_ROT_LATENCY 2.5     // Avg Rotational Latency; 2.5 ms for 180 degrees
#define TRANSFER_RATE 6         // 6 GB/s
#define BLOCK_SIZE 4            // 4 KB
#define RPM 12000               // Rotations per minute
#define RPS 200                 // Rotations per second
#define RPMS 0.2                // Rotations per millisecond
#define MS_PER_SECT 0.000556    // Millisecond per sector
#define TRACKS 201
#define SECTORS 360

/* Struct holding the info (results) for one experiment */
struct hddSim
{
    float avgSeekLength;        // Average Seek Time
    int totalRequests;          // Total number of requests
    float totalTime;            // Total time duration of simulation
    float totalAvgAccessTime;   // Total average access time

    /* Unused vars */
    //float avgRequestT;        // Average Request Time
    //float avgRotDelay;        // Average Rotational Delay
    //float avgAccessT;         // Average Access Time
    //int totalBytes;           // Total number of bytes transferred
};

/* Struct holding one algorithm's overall results for a batch of tests */
struct results
{
    float totalAvgAccessTime;   // Total Average Access Time
    float avgReqTime;           // Average Request Time
    int totalReq;               // Total number of requests
};

/* Define methods */
vector<results> executeAlg(char alg);
hddSim fifo(vector<ioReq> req);
hddSim sstf(vector<ioReq> req);
hddSim scan(vector<ioReq> req);
hddSim lifo(vector<ioReq> req);
vector<ioReq> generateRequests(int n, unsigned seed);
void printResults(vector<results> res);
void updateProgressBar(int p, int total, char alg);
int partitionBySector(vector<ioReq> &values, int left, int right);
int partitionByTrack(vector<ioReq> &req, int left, int right);
void quicksort(vector<ioReq> &values, int left, int right, string comparison);


/*
    int main()
    author: Gherkin
    date: Nov 22, 2020
    description: Manages the calls for the testing and printing methods.
*/
int main()
{
    vector<results> fifoResults = executeAlg('a');
    vector<results> sstfResults = executeAlg('b');
    vector<results> scanResults = executeAlg('c');
    vector<results> lifoResults = executeAlg('d');

    cout << "FIFO Results:\n";
    printResults(fifoResults);
    cout << "SSTF Results:\n";
    printResults(sstfResults);
    cout << "SCAN Results:\n";
    printResults(scanResults);
    cout << "LIFO Results:\n";
    printResults(lifoResults);

    return 0;
}

/*
    vector<results> executeAlg(char alg)
    author: Gherkin
    date: Nov 22, 2020
    description: Manages the testing conditions and execution of the scheduling algorithms.
    parameters:
        alg         I/P  char             character indicating which alg to execute
        executeAlg  O/P  vector<results>  Vector containing the results for all tests
*/
vector<results> executeAlg(char alg)
{
    // start clock
    typedef std::chrono::high_resolution_clock clock;
    clock::time_point start_point = clock::now();

    vector<results> algRes;

    // testing range of 50 to 150 I/O requests, incrementing in steps of 10
    for (int io = 50; io <= 150; io += 10)
    {
        results batchRes; // set up collection results struct
        batchRes.avgReqTime = 0;
        batchRes.totalReq = 0;

        // conduct 1000 trials and report the averaged results
        for (int e = 0; e < EXPERIMENTS; e++)
        {
            // create randomization seed using the duration from the clock's staring point
            clock::duration d = clock::now() - start_point;
            unsigned seed = d.count();

            vector<ioReq> requests = generateRequests(io, seed); // generate vector of I/O requests
            hddSim s;

            switch (alg)
            {
            case 'a':
                s = fifo(requests);
                break;
            case 'b':
                s = sstf(requests);
                break;
            case 'c':
                s = scan(requests);
                break;
            case 'd':
                s = lifo(requests);
                break;
            }

            batchRes.totalAvgAccessTime += s.totalAvgAccessTime;
            batchRes.avgReqTime += s.totalTime;
            batchRes.totalReq += io;
        }

        // convert values into averages
        batchRes.totalAvgAccessTime /= EXPERIMENTS;
        batchRes.avgReqTime /= EXPERIMENTS;
        batchRes.totalReq /= EXPERIMENTS;

        algRes.push_back(batchRes); // add batch results to total results vector
        updateProgressBar(io - 50, 100, alg);
    }
    return algRes;
}

/*
    int updateSector(float currentTime)
    author: Gherkin
    date: Nov 22, 2020
    description: Produce an updated sector value based on the given time value
    parameters:
        currentTime   I/P  float  Time value used to calculate sector position
        updateSector  O/P  int    Updated value of sector position
*/
int updateSector(float currentTime)
{
    int s = currentTime * (RPMS / 360);
    s -= (360 * (s / 360));
    s -= 1;
    return s;
}

/*
    int updateSector(int sector, int duration)
    author: Gherkin
    date: Nov 22, 2020
    description: Increased the current sector value based on the given time value
    parameters:
        sector        I/P  int  Current sector position value
        duration      I/P  int  Duration of time
        updateSector  O/P  int  Increased value of sector position
*/
int updateSector(int sector, int duration)
{
    int sec = sector;
    for (int i = 0; i < duration; i++)
    {
        sec++;
        if (sec == 360)
            sec = 0;
    }
    return sec;
}

/*
    hddSim fifo(vector<ioReq> req)
    author: Gherkin
    date: Nov 22, 2020
    description: Emulates the performance of a First-In-First-Out disk scheduling algorithm.
    parameters:
        req   I/P  vector<ioReq>  Vector of simulated I/O requests
        fifo  O/P  hddSim         Struct containing simulation statistics
*/
hddSim fifo(vector<ioReq> req)
{
    int dhTrack = 100; // Disk head track starts at 100 for each experiment
    int dhSector = 0;  // Disk head sector starts at 0 for each experiment
    hddSim sim;
    sim.totalRequests = req.size();
    sim.totalTime = 0;
    sim.avgSeekLength = 0;

    for (ioReq r : req)
    {
        // switch tracks if needed
        int distance = abs(r.track - dhTrack);
        float duration = (float)distance * AVG_SEEK_TIME;
        sim.avgSeekLength += distance;
        sim.totalTime += duration;

        dhTrack = r.track;                      // set dhTrack to the destination value
        dhSector = updateSector(sim.totalTime); // accounting for disk spin while seeking

        // calculate rotational latency
        int sectDiff = 0;
        if (r.sector >= dhSector)
            sectDiff = r.sector - dhSector;
        else
            sectDiff = (359 - dhSector) + r.sector; // the remaining distance on the disk plus the additional rotation
        dhSector = updateSector(dhSector, sectDiff);
        sim.totalTime += sectDiff * (RPMS / 360);

        // calculate transfer time
        float transferTime = (float)(BLOCK_SIZE * 1024) / (float)6000000000;
        transferTime *= 1000;
        sim.totalTime += transferTime;

        float totalAvgAccessTime = AVG_SEEK_TIME + (1 / (2 * RPM)) + transferTime;
        sim.totalAvgAccessTime += totalAvgAccessTime;
    }

    sim.totalAvgAccessTime /= req.size();
    sim.avgSeekLength /= req.size();
    return sim;
}

/*
    hddSim sstf(vector<ioReq> req)
    author: Gherkin
    date: Nov 22, 2020
    description: Emulates the performance of a Shortest Service Time First disk scheduling algorithm.
    parameters:
        req   I/P  vector<ioReq>  Vector of simulated I/O requests
        sstf  O/P  hddSim         Struct containing simulation statistics
*/
hddSim sstf(vector<ioReq> req)
{
    int dhTrack = 100; // Disk head track starts at 100 for each experiment
    int dhSector = 0;  // Disk head sector starts at 0 for each experiment
    int rSize = req.size();
    hddSim sim;
    sim.totalRequests = rSize;
    sim.totalTime = 0;

    bool ascendingStart = true;
    bool ascended = false, descended = false;

    // sort request vector then group by tracks
    quicksort(req, 0, rSize, "sector");
    quicksort(req, 0, rSize, "track");

    // find best starting point
    int startIndex = 0;
    float startTime = 999;
    for (int x = 0; x < rSize; x++) {
        // find duration of track traversal
        int distance = abs(req[x].track - dhTrack);
        float duration = (float)distance * AVG_SEEK_TIME;
        int futureSector = updateSector(duration);
        float futureTime = duration;
        
        // account for rotational latency
        int sectDiff = 0;
        if (req[x].sector >= futureSector)
            sectDiff = req[x].sector - futureSector;
        else
            sectDiff = (359 - futureSector) + req[x].sector;

        futureTime += sectDiff * (RPMS / 360);

        // if evaluated potential time is better than current best
        if (futureTime < startTime)
        {
            startIndex = x;
            startTime = duration;
        }
    }

    // if write head will start in decending order
    if (req[startIndex].track < dhTrack)
        ascendingStart = false;

    // HUB section determines the control flow
    HUB:
    if (ascendingStart) // if drive head starts in an ascending direction
    {
        if (!ascended)
            goto ASCEND;
        else if (!descended)
            goto DESCEND;
        else
            goto END;
    }
    else // if drive head starts in an descending direction
    {
        if (!descended)
            goto DESCEND;
        else if (!ascended)
            goto ASCEND;
        else
            goto END;
    }
    
    ASCEND:
    // head is moving in ascending order
    for (int t = startIndex; t < rSize; t++)
    {
        // check for track switches
        if (req[t].track != dhTrack)
        {
            int distance = abs(req[t].track - dhTrack);
            float duration = (float)distance * AVG_SEEK_TIME;
            sim.avgSeekLength += distance;
            sim.totalTime += duration;

            dhTrack = req[t].track;                 // set dhTrack to the destination value
            dhSector = updateSector(sim.totalTime); // accounting for disk spin while seeking
        }

        ioReq r = req[t];

        // calculate rotational latency
        int sectDiff = 0;
        if (r.sector >= dhSector)
            sectDiff = r.sector - dhSector;
        else
            sectDiff = (359 - dhSector) + r.sector; // the remaining distance on the disk plus the additional rotation
        dhSector = updateSector(dhSector, sectDiff);
        sim.totalTime += sectDiff * (RPMS / 360);

        // calculate transfer time
        float transferTime = (float)(BLOCK_SIZE * 1024) / (float)6000000000;
        transferTime *= 1000;
        sim.totalTime += transferTime;

        // calculate access time
        float totalAvgAccessTime = AVG_SEEK_TIME + (1 / (2 * RPM)) + transferTime;
        sim.totalAvgAccessTime += totalAvgAccessTime;
    }
    ascended = true;
    goto HUB; // return to HUB

    DESCEND:
    // head traverses in reverse order
    for (int t = startIndex; t >= 0; t--)
    {
        // check for track switches
        if (req[t].track != dhTrack)
        {
            int distance = abs(req[t].track - dhTrack);
            float duration = (float)distance * AVG_SEEK_TIME;
            sim.avgSeekLength += distance;
            sim.totalTime += duration;

            dhTrack = req[t].track;                 // set dhTrack to the destination value
            dhSector = updateSector(sim.totalTime); // accounting for disk spin while seeking
        }

        ioReq r = req[t];

        // calculate rotational latency
        int sectDiff = 0;
        if (r.sector >= dhSector)
            sectDiff = r.sector - dhSector;
        else
            sectDiff = (359 - dhSector) + r.sector; // the remaining distance on the disk plus the additional rotation
        dhSector = updateSector(dhSector, sectDiff);
        sim.totalTime += sectDiff * (RPMS / 360);

        // calculate transfer time
        float transferTime = (float)(BLOCK_SIZE * 1024) / (float)6000000000;
        transferTime *= 1000;
        sim.totalTime += transferTime;

        // calculate access time
        float totalAvgAccessTime = AVG_SEEK_TIME + (1 / (2 * RPM)) + transferTime;
        sim.totalAvgAccessTime += totalAvgAccessTime;
    }
    descended = true;
    goto HUB; // return to HUB

    END:
    sim.totalAvgAccessTime /= req.size();
    sim.avgSeekLength /= req.size();
    return sim;
}

/*
    hddSim scan(vector<ioReq> req)
    author: Gherkin
    date: Nov 22, 2020
    description: Emulates the performance of a SCAN disk scheduling algorithm.
    parameters:
        req   I/P  vector<ioReq>  Vector of simulated I/O requests
        scan  O/P  hddSim         Struct containing simulation statistics
*/
hddSim scan(vector<ioReq> req)
{
    int dhTrack = 100; // Disk head track starts at 100 for each experiment
    int dhSector = 0;  // Disk head sector starts at 0 for each experiment
    int rSize = req.size();
    hddSim sim;
    sim.totalRequests = rSize;
    sim.totalTime = 0;

    // sort request vector then group by tracks
    quicksort(req, 0, rSize, "sector");
    quicksort(req, 0, rSize, "track");

    // find the first value in track 100. Not very optimized.
    int startIndex = 0;
    for (int x = 0; x < rSize; x++)
    {
        if (req[x].track >= dhTrack)
        {
            startIndex = x;
            break;
        }
    }

    // head is moving ascending order first
    for (int t = startIndex; t < rSize; t++)
    {
        // check for track switches
        if (req[t].track != dhTrack)
        {
            int distance = abs(req[t].track - dhTrack);
            float duration = (float)distance * AVG_SEEK_TIME;
            sim.avgSeekLength += distance;
            sim.totalTime += duration;

            dhTrack = req[t].track;                 // set dhTrack to the destination value
            dhSector = updateSector(sim.totalTime); // accounting for disk spin while seeking
        }

        ioReq r = req[t];

        // calculate rotational latency
        int sectDiff = 0;
        if (r.sector >= dhSector)
            sectDiff = r.sector - dhSector;
        else
            sectDiff = (359 - dhSector) + r.sector; // the remaining distance on the disk plus the additional rotation
        dhSector = updateSector(dhSector, sectDiff);
        sim.totalTime += sectDiff * (RPMS / 360);

        // calculate transfer time
        float transferTime = (float)(BLOCK_SIZE * 1024) / (float)6000000000;
        transferTime *= 1000;
        sim.totalTime += transferTime;

        // calculate access time
        float totalAvgAccessTime = AVG_SEEK_TIME + (1 / (2 * RPM)) + transferTime;
        sim.totalAvgAccessTime += totalAvgAccessTime;
    }

    // head traverses back in reverse order
    for (int t = startIndex; t >= 0; t--)
    {
        // check for track switches
        if (req[t].track != dhTrack)
        {
            int distance = abs(req[t].track - dhTrack);
            float duration = (float)distance * AVG_SEEK_TIME;
            sim.avgSeekLength += distance;
            sim.totalTime += duration;

            dhTrack = req[t].track;                 // set dhTrack to the destination value
            dhSector = updateSector(sim.totalTime); // accounting for disk spin while seeking
        }

        ioReq r = req[t];

        // calculate rotational latency
        int sectDiff = 0;
        if (r.sector >= dhSector)
            sectDiff = r.sector - dhSector;
        else
            sectDiff = (359 - dhSector) + r.sector; // the remaining distance on the disk plus the additional rotation
        dhSector = updateSector(dhSector, sectDiff);
        sim.totalTime += sectDiff * (RPMS / 360);

        // calculate transfer time
        float transferTime = (float)(BLOCK_SIZE * 1024) / (float)6000000000;
        transferTime *= 1000;
        sim.totalTime += transferTime;

        // calculate access time
        float totalAvgAccessTime = AVG_SEEK_TIME + (1 / (2 * RPM)) + transferTime;
        sim.totalAvgAccessTime += totalAvgAccessTime;
    }

    sim.totalAvgAccessTime /= req.size();
    sim.avgSeekLength /= req.size();
    return sim;
}

/*
    hddSim lifo(vector<ioReq> req)
    author: Gherkin
    date: Nov 22, 2020
    description: Emulates the performance of a Last-In-First-Out disk scheduling algorithm.
    parameters:
        req   I/P  vector<ioReq>  Vector of simulated I/O requests
        lifo  O/P  hddSim         Struct containing simulation statistics
*/
hddSim lifo(vector<ioReq> req)
{
    int dhTrack = 100; // Disk head track starts at 100 for each experiment
    int dhSector = 0;  // Disk head sector starts at 0 for each experiment
    hddSim sim;
    int rSize = req.size();
    sim.totalRequests = req.size();
    sim.totalTime = 0;
    sim.avgSeekLength = 0;

    while (req.size() != 0)
    {
        ioReq r = req.back();

        // switch tracks if needed
        int distance = abs(r.track - dhTrack);
        float duration = (float)distance * AVG_SEEK_TIME;
        sim.avgSeekLength += distance;
        sim.totalTime += duration;

        dhTrack = r.track;                      // set dhTrack to the destination value
        dhSector = updateSector(sim.totalTime); // accounting for disk spin while seeking

        // calculate rotational latency
        int sectDiff = 0;
        if (r.sector >= dhSector)
            sectDiff = r.sector - dhSector;
        else
            sectDiff = (359 - dhSector) + r.sector; // the remaining distance on the disk plus the additional rotation
        dhSector = updateSector(dhSector, sectDiff);
        sim.totalTime += sectDiff * (RPMS / 360);

        // calculate transfer time
        float transferTime = (float)(BLOCK_SIZE * 1024) / (float)6000000000;
        transferTime *= 1000;
        sim.totalTime += transferTime;

        // calculate access time
        float totalAvgAccessTime = AVG_SEEK_TIME + (1 / (2 * RPM)) + transferTime;
        sim.totalAvgAccessTime += totalAvgAccessTime;

        req.pop_back();
    }

    sim.totalAvgAccessTime /= rSize;
    sim.avgSeekLength /= rSize;
    return sim;
}

/*
    vector<ioReq> generateRequests(n, seed)
    author: Gherkin
    date: Nov 22, 2020
    description: Randomly generate a collection of simulated I/O requests.
    parameters:
        n                 I/P  int            Number of requests to generate
        seed              I/P  unsigned       Randomization seed
        generateRequests  O/P  vector<ioReq>  Vector of simulated I/O requests 
*/
vector<ioReq> generateRequests(int n, unsigned seed)
{
    vector<ioReq> requests;
    default_random_engine generator;
    generator.seed(seed);
    uniform_int_distribution<int> trackDistr(0, TRACKS - 1);
    uniform_int_distribution<int> sectorDistr(0, SECTORS - 1);

    for (int k = 0; k < n; k++)
    {
        int t = trackDistr(generator);
        int s = sectorDistr(generator);
        ioReq *r = new ioReq(t, s);
        requests.push_back(*r);
    }

    return requests;
}

/*
    void printResults(res)
    author: Gherkin
    date: Nov 22, 2020
    description: Apply stylization and print algorithm test results to the console.
    parameters:
        res  I/P  vector<results>  All testing results for one algorithm
*/
void printResults(vector<results> res)
{
    cout << " T#  |  Avg Req Time |  Requests   |    Avg Access Time \n";
    for (int c = 0; c < res.size(); c++)
    {
        string num = "[0" + to_string(c) + "]\t";
        if (c >= 10)
            num = "[" + to_string(c) + "]\t";
        cout << num;
        //cout << to_string(res[c].avgReqTime) + " ms  \t";
        printf("%.3f ms\t",res[c].avgReqTime);
        cout << to_string(res[c].totalReq) + " req  \t";
        cout << to_string(res[c].totalAvgAccessTime) + " ms\n";
    }
    cout << endl;
}

/*
    void updateProgressBar(p, total, alg)
    author: Gherkin
    date: Nov 22, 2020
    description: Print a graphical indicator of the current test's progression.
    parameters:
        p      I/P  int   Progression of execution loop
        total  I/P  int   End point of loop
        alg    I/P  char  Character representation of current algorithm

*/
void updateProgressBar(int p, int total, char alg)
{
    float progress = (float)p / total;
    if (progress == 1)
    {
        cout << "Algorithm " << (char)toupper(alg) << " complete.              \n\n";
        return;
    }

    else
    {
        int barWidth = 30;
        int pos = barWidth * progress;
        //cout << "[";

        char f = 178, e = 176;

        for (int i = 0; i < barWidth; ++i)
        {
            if (i <= pos)
                cout << f;
            else
                cout << '_';
        }
        cout << " " << int(progress * 100.0) << " %\r";
        cout.flush();

        if (progress == 1)
            cout << endl
                 << endl;
    }
}

/*
    void quicksort(&req, left, right, comparison)
    author: Gherkin
    date: Nov 22, 2020
    description: Use a Quicksort algorithm to sort I/O requests by either Sector or Track values.
    parameters:
        &req        I/P  vector<ioReq>  Vector of I/O requests to be sorted
        left        I/P  int            Left most point of vector
        right       I/P  int            Right most point of vector
        comparison  I/P  string         Indicating what value should be compared

*/
void quicksort(vector<ioReq> &req, int left, int right, string comparison)
{
    if (left < right)
    {
        int pivot;
        if (comparison == "sector")
            pivot = partitionBySector(req, left, right);
        else
            pivot = partitionByTrack(req, left, right);

        quicksort(req, left, pivot - 1, comparison);
        quicksort(req, pivot, right, comparison);
    }
}

/*
    int partitionBySector(&req, left, right)
    author: Gherkin
    date: Nov 22, 2020
    description: Quicksort partitioning through comparing Sector values of each I/O request.
    parameters:
        &req               I/P  vector<ioReq>  Vector of I/O requests to be sorted
        left               I/P  int            Left most point of vector
        right              I/P  int            Right most point of vector
        partitionBySector  O/P  int            Identified pivot point
*/
int partitionBySector(vector<ioReq> &req, int left, int right)
{
    ioReq temp;
    int l = left, r = right;

    int pivot = left + (right - left) / 2;
    int pivotValue = req[pivot].sector;

    while (l <= r)
    {
        while (req[l].sector < pivotValue)
            l++;
        while (req[r].sector > pivotValue)
            r--;

        if (l <= r)
        {
            temp = req[l];
            req[l] = req[r];
            req[r] = temp;
            l++;
            r--;
        }
    }
    return l;
}

/*
    int partitionByTrack(&req, left, right)
    author: Gherkin
    date: Nov 22, 2020
    description: Quicksort partitioning through comparing Track values of each I/O request.
    parameters:
        &req              I/P  vector<ioReq>  Vector of I/O requests to be sorted
        left              I/P  int            Left most point of vector
        right             I/P  int            Right most point of vector
        partitionByTrack  O/P  int            Identified pivot point
*/
int partitionByTrack(vector<ioReq> &req, int left, int right)
{
    ioReq temp;
    int l = left, r = right;

    int pivot = left + (right - left) / 2;
    int pivotValue = req[pivot].track;

    while (l <= r)
    {
        while (req[l].track < pivotValue)
            l++;
        while (req[r].track > pivotValue)
            r--;

        if (l <= r)
        {
            temp = req[l];
            req[l] = req[r];
            req[r] = temp;
            l++;
            r--;
        }
    }
    return l;
}