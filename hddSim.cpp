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

/*
    HDD:
    RPM = 12,000 (200 rotations per second)
    Avg Seek Time = 2.5 ms (time needed to move the head between tracks)
    Avg Rotational Latency = 2.5 ms for 180 degrees
    Transfer Rate = 6 GB/s (in powers of 10?)
    Tracks = 201
    Sectors per track = 360
    Block size = 4 KB (in powers of 2?)
*/

// study algs for 50 to 150 I/O requests, counting by steps of 10
// disk head resets to track 100, sector 0 before each experiment
// track and sector are generated using uniform rand distribution
// min 1000 experiments per I/O request step
// chart everything on a graph afterwards
// assume one sector is written for each request

#define EXPERIMENTS 1000
#define AVG_SEEK_TIME 2.5
#define AVG_ROT_LATENCY 2.5
#define TRANSFER_RATE 6 // 6 GB/s
#define BLOCK_SIZE 4    // 4 KB
#define RPM 12000
#define RPS 200  //Rotations per second
#define RPMS 0.2 //Rotations per millisecond
#define MS_PER_SECT 0.000556
#define TRACKS 201
#define SECTORS 360

// holds the info (results) for one experiment
struct hddSim
{
    float avgRequestT;   // Average Request Time
    float avgSeekLength; // Average Seek Time
    float avgRotDelay;   // Average Rotational Delay
    float avgAccessT;    // Average Access Time
    int totalBytes;      // Total number of bytes transfered
    int totalRequests;   // Total number of requests
    float totalTime;
    float totalAvgAccessTime;
};

// holds overall testing results for a disk scheduling policy
struct results
{
    float avgReqTime; // Average Request Time
    int totalReq;     // Total number of requests
};

vector<results> runAlg(char alg);
hddSim fifo(vector<ioReq> req);
hddSim sstf(vector<ioReq> req);
hddSim scan(vector<ioReq> req);
hddSim lifo(vector<ioReq> req);
vector<ioReq> generateRequests(int n, unsigned seed);
void printResults(vector<results> res);
void updateProgressBar(int p, int total, int update);

int partition(vector<ioReq> &values, int left, int right);
void quicksort(vector<ioReq> &values, int left, int right);
vector<vector<ioReq>> splitTracks(vector<ioReq> values);

int main()
{
    // typedef std::chrono::high_resolution_clock clock;
    // clock::time_point start_point = clock::now();
    // clock::duration d = clock::now() - start_point;
    // unsigned seed = d.count();
    // vector<ioReq> requests = generateRequests(150, seed);

    // cout << "original: ";
    // for (int i = 0; i < requests.size(); ++i)
    //     cout << to_string(requests[i].sector) + " ";

    // quicksort(requests, 0, requests.size());

    // cout << "\nsorted: ";
    // for (int i = 0; i < requests.size(); ++i)
    //     cout << to_string(requests[i].sector) + " ";

    // vector<vector<ioReq>> sortedReq = splitTracks(requests);
    // cout << "\nsorted: \n";
    // for (int i = 0; i < sortedReq.size(); ++i)
    // {
    //     cout << "track " + to_string(i) + ": ";
    //     for (int j = 0; j < sortedReq[i].size(); ++j)
    //     {
    //         cout << to_string(sortedReq[i][j].sector) + " ";
    //     }
    //     cout << endl;
    // }

    vector<results> fifoResults = runAlg('a');
    vector<results> sstfResults = runAlg('b');
    //vector<results> scanResults = runAlg('c');
    vector<results> lifoResults = runAlg('d');

    printResults(fifoResults);
    printResults(sstfResults);
    printResults(lifoResults);

    return 0;
}

vector<results> runAlg(char alg)
{
    typedef std::chrono::high_resolution_clock clock;
    clock::time_point start_point = clock::now();
    vector<results> algRes;

    for (int io = 50; io <= 150; io += 10)
    {
        results batchRes; // set up collection results struct
        batchRes.avgReqTime = 0;
        batchRes.totalReq = 0;

        for (int e = 0; e < EXPERIMENTS; e++)
        {
            clock::duration d = clock::now() - start_point;
            unsigned seed = d.count();
            vector<ioReq> requests = generateRequests(io, seed);
            hddSim s;

            switch (alg)
            {
            case 'a':
                s = fifo(requests);
                break;
            case 'b':
                s = sstf(requests);
                break;
            case 'd':
                s = scan(requests);
                break;
            case 'e':
                s = lifo(requests);
                break;
            default:
                break;
            }

            batchRes.avgReqTime += s.totalTime / io;
            batchRes.totalReq += io;
        }

        // convert values into averages
        batchRes.avgReqTime /= EXPERIMENTS;
        batchRes.totalReq /= EXPERIMENTS; // I'm not sure if I need to average this one

        algRes.push_back(batchRes); // add batch results to total results vector
        updateProgressBar(io - 50, 100, 1);
    }
    return algRes;
}

int updateSector(float currentTime)
{
    int s = currentTime * (RPMS / 360);
    s -= (360 * (s / 360));
    s -= 1;
    //cout << "sector updated: " + to_string(s) + "\n";
    return s;
}

int updateSector(int sector, int duration)
{
    int sec = sector;
    for (int i = 0; i < duration; i++)
    {
        sec++;
        if (sec == 360)
            sec = 0;
    }
    //cout << "sector increased: " + to_string(sector) + "\n";
    return sec;
}

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

        // calculate rotaional latency
        int sectDiff = 0;
        if (r.sector >= dhSector)
            sectDiff = r.sector - dhSector;
        else
            sectDiff = (359 - dhSector) + r.sector; // the remaining distance on the disk plus the addional rotation
        dhSector = updateSector(dhSector, sectDiff);
        sim.totalTime += sectDiff * (RPMS / 360);

        // calulate transfer time
        float transferTime = BLOCK_SIZE * TRANSFER_RATE * 1000; // GB/s * 1000 coverts to KB/ms
        sim.totalTime += transferTime;

        float totalAvgAccessTime = AVG_SEEK_TIME + (1 / (2 * RPM)) + transferTime;
        /* I'm not sure how to use this but people in the groupme said it was important. */
        sim.totalAvgAccessTime += totalAvgAccessTime;
    }

    sim.totalAvgAccessTime /= req.size();
    sim.avgSeekLength /= req.size();
    return sim;
}

hddSim sstf(vector<ioReq> req)
{
    int dhTrack = 100; // Disk head track starts at 100 for each experiment
    int dhSector = 0;  // Disk head sector starts at 0 for each experiment
    hddSim sim;
    sim.totalRequests = req.size();
    sim.totalTime = 0;

    // sort request vector then group by tracks
    quicksort(req, 0, req.size());
    vector<vector<ioReq>> sortedReq = splitTracks(req);

    // head is moving assending order first
    for (int t = 100; t < TRACKS; t++)
    {
        // skip track vectors with no IO requests
        if (sortedReq[dhTrack].size() == 0)
            continue;

        // switch tracks
        int distance = abs(t - dhTrack);
        float duration = (float)distance * AVG_SEEK_TIME;
        sim.avgSeekLength += distance;
        sim.totalTime += duration;

        for (int s = 0; s < sortedReq[dhTrack].size(); s++)
        {
            ioReq r = sortedReq[dhTrack][s];

            // move to next sector & calculate rotaional latency
            int sectDiff = r.sector - dhSector;
            dhSector = updateSector(dhSector, sectDiff);
            sim.totalTime += sectDiff * (RPMS / 360);

            // calulate transfer time
            float transferTime = BLOCK_SIZE * TRANSFER_RATE * 1000; // GB/s * 1000 coverts to KB/ms
            sim.totalTime += transferTime;

            // calculate access time
            float totalAvgAccessTime = AVG_SEEK_TIME + (1 / (2 * RPM)) + transferTime;
            sim.totalAvgAccessTime += totalAvgAccessTime;
        }
    }

    sim.totalAvgAccessTime /= req.size();
    sim.avgSeekLength /= req.size();
    return sim;
}

hddSim scan(vector<ioReq> req)
{
    hddSim sim;
    return sim;
    // sort the list
    // start at t100, move towards increasing tracks. Once done, start moving towards decreasing tracks.
}

hddSim lifo(vector<ioReq> req)
{
    int dhTrack = 100; // Disk head track starts at 100 for each experiment
    int dhSector = 0;  // Disk head sector starts at 0 for each experiment
    hddSim sim;
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

        // calculate rotaional latency
        int sectDiff = 0;
        if (r.sector >= dhSector)
            sectDiff = r.sector - dhSector;
        else
            sectDiff = (359 - dhSector) + r.sector; // the remaining distance on the disk plus the addional rotation
        dhSector = updateSector(dhSector, sectDiff);
        sim.totalTime += sectDiff * (RPMS / 360);

        // calulate transfer time
        float transferTime = BLOCK_SIZE * TRANSFER_RATE * 1000; // GB/s * 1000 coverts to KB/ms
        sim.totalTime += transferTime;

        float totalAvgAccessTime = AVG_SEEK_TIME + (1 / (2 * RPM)) + transferTime;
        /* I'm not sure how to use this but people in the groupme said it was important. */
        sim.totalAvgAccessTime += totalAvgAccessTime;

        req.pop_back();
    }

    sim.totalAvgAccessTime /= req.size();
    sim.avgSeekLength /= req.size();
    return sim;
}

vector<ioReq> generateRequests(int n, unsigned seed)
{
    vector<ioReq> requests;
    default_random_engine generator;
    // get random generation seed
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

void printResults(vector<results> res)
{
    cout << "[T#]    [Avg Req Time (ms)]    [# Requests]\n";
    for (int c = 0; c < res.size(); c++)
    {
        string num = "[0" + to_string(c) + "]\t";
        if (c >= 10)
            num = "[" + to_string(c) + "]\t";
        cout << num;
        cout << to_string(res[c].avgReqTime) + " ms  \t";
        cout << to_string(res[c].totalReq) + " req\n";
    }
    cout << endl;
}

void updateProgressBar(int p, int total, int update)
{
    float progress = (float)p / total;

    if (progress != 1 && (p % update != 0))
        return;
    else
    {
        int barWidth = 50;
        int pos = barWidth * progress;
        cout << "[";

        for (int i = 0; i < barWidth; ++i)
        {
            if (i <= pos)
                cout << "#";
            else
                cout << "_";
        }
        cout << "] " << int(progress * 100.0) << " %\r";
        cout.flush();

        if (progress == 1)
            cout << endl
                 << endl;
    }
}

//// THIS CODE NEEDS TO BE REPLACED ////
void quicksort(vector<ioReq> &values, int left, int right)
{
    if (left < right)
    {
        int pivotIndex = partition(values, left, right);
        quicksort(values, left, pivotIndex - 1);
        quicksort(values, pivotIndex, right);
    }
}

int partition(vector<ioReq> &values, int left, int right)
{
    int pivotIndex = left + (right - left) / 2;
    int pivotValue = values[pivotIndex].sector;
    int i = left, j = right;
    ioReq temp;
    while (i <= j)
    {
        while (values[i].sector < pivotValue)
        {
            i++;
        }
        while (values[j].sector > pivotValue)
        {
            j--;
        }
        if (i <= j)
        {
            temp = values[i];
            values[i] = values[j];
            values[j] = temp;
            i++;
            j--;
        }
    }
    return i;
}

vector<vector<ioReq>> splitTracks(vector<ioReq> values)
{
    // construct output vector
    vector<vector<ioReq>> v;
    vector<ioReq> track;
    for (int t = 0; t < TRACKS; t++)
        v.push_back(track);

    // sort values into appropriate tracks
    for (ioReq r : values)
    {
        v[r.track].push_back(r);
    }

    return v;
}