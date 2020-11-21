#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include "ioReq.cpp"

using namespace std;

/*
    HDD:
    RPM = 12,000
    Avg Seek Time = 2.5 ms (time needed to move the head between tracks)
    Transfer Rate = 6 GB/s
    Tracks = 201
    Sectors per track = 360
    Block size = 4 KB
*/

// study algs for 50 to 150 I/O requests, counting by steps of 10
// disk head resets to track 100, sector 0 before each experiment
// track and sector are generated using uniform rand distribution
// min 1000 experiments per I/O request step
// chart everything on a graph afterwards

#define EXPERIMENTS 1000

hddSim fifo(vector<ioReq> req);
hddSim sstf(vector<ioReq> req);
hddSim scan(vector<ioReq> req);
hddSim lifo(vector<ioReq> req);

// holds the info (results) for one experiment
struct hddSim
{
    float avgRequestT;  // Average Request Time
    float avgSeekT;     // Average Seek Time
    float avgRotDelay;  // Average Rotational Delay
    float avgAccessT;   // Average Access Time
    int totalBytes;     // Total number of bytes transfered
    int totalRequests;  // Total number of requests
};

// holds overall testing results for a disk scheduling policy
struct results
{
    float avgReqTime;  // Average Request Time
    int totalReq;  // Total number of requests
};

int main(){
    
    results fifoRes;
    fifoRes.avgReqTime = 0;
    fifoRes.totalReq = 0;

    for (int io = 50; io <= 150; io + 10){
        for (int e = 0; e < EXPERIMENTS; e++){
            vector<ioReq> requests = generateRequests(io);
            hddSim s = fifo(requests);
        }
    }

    return 0;
}

vector<ioReq> generateRequests(int n){
    vector<ioReq> requests;
    default_random_engine generator;
    // get random generation seed
    // generator.seed();
    uniform_int_distribution<int> trackDistr(0,200);
    uniform_int_distribution<int> sectorDistr(0,359);

    for (int k = 0; k < n; k++){
        int t = trackDistr(generator);
        int s = sectorDistr(generator);
        ioReq *r = new ioReq(t,s);
        requests.push_back(*r);
    }

    return requests;
}

hddSim fifo(vector<ioReq> req){
    int dhTrack = 100; // Disk head track starts at 100 for each experiment
    int dhSector = 0;  // Disk head sector starts at 0 for each experiment
    hddSim simulation;
    simulation.totalRequests = req.size();

    for (ioReq request : req){
        /* perform algorithm */
    }
}

hddSim sstf(vector<ioReq> req){

}

hddSim scan(vector<ioReq> req){

}

hddSim lifo(vector<ioReq> req){

}
