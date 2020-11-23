## OS Concepts Programming Assignment 5

### Problem Statement
The task was to perform a Monte Carlo simulation of multiple disk scheduling algorithms within the environment of a hypothetical HDD. This disk drive was given the following specifications: 12,000 RPM, 2.5 ms average seek time, 6 GB/s transfer rate, 4 KB block size, 201 tracks, and 360 sectors per track. Based on these specifications, the program should emulate the performance of the First-In-First-Out, Shortest Service Time First, SCAN, and Last-In-First-Out scheduling algorithms.
Experiments should be done with simulated I/O requests. Each request should have its track and sector value generated from a uniform random distribution. The number of I/O requests received will range from 50 to 150, increasing by 10 between each test. Every test consists of 1000 experiments, with the test results being drawn from the average of these experiments.

### Approach
I developed the program primarily within Windows using the MinGW C++ compiler, then completed the final portion of the program on a Linux Ubuntu virtual machine. Time emulation was done using a float value to represent the total duration in milliseconds. I began by designing and coding the basic data collection functionality, then moved on to implementing the algorithms. Testing data and results are stored using structs in order to simplify the process of transferring information between methods. I/O requests are stored using a separate dedicated class.

### Solution
Within each test, I/O request completion time is calculated by combining the time spent seeking, delay caused by rotational latency, and time needed to transfer the simulated data. The average request completion time is using all the requests processes within one experiment. The average is then calculated from all 1000 experiments to produce the Average Request Time displayed in the output. Additional factors, such as the time needed to perform the searching logic with SSTF, are not included in the simulated timing calculations. 

### Build and Execution
```console
gherkin@Gherkin-VM:~/Documents/pgm5$ g++ hddSim.cpp -o hddSim
gherkin@Gherkin-VM:~/Documents/pgm5$ ./hddSim
Algorithm A complete.

Algorithm B complete.

Algorithm C complete.

Algorithm D complete.

FIFO Results:
 T#  |  Avg Req Time |  Requests   |    Avg Access Time 
[00]    85.995 ms       50 req          0.026079 ms     
[01]    102.666 ms      60 req          0.026017 ms     
[02]    121.789 ms      70 req          0.025954 ms     
[03]    144.905 ms      80 req          0.025907 ms     
[04]    158.808 ms      90 req          0.025871 ms     
[05]    177.056 ms      100 req         0.025842 ms     
[06]    191.939 ms      110 req         0.025818 ms     
[07]    214.579 ms      120 req         0.025798 ms     
[08]    227.156 ms      130 req         0.025781 ms
[09]    248.258 ms      140 req         0.025767 ms
[10]    264.136 ms      150 req         0.025755 ms

SSTF Results:
 T#  |  Avg Req Time |  Requests   |    Avg Access Time
[00]    12.315 ms       50 req          0.026626 ms
[01]    13.282 ms       60 req          0.026451 ms
[02]    14.462 ms       70 req          0.026325 ms
[03]    15.377 ms       80 req          0.026231 ms
[04]    16.461 ms       90 req          0.026158 ms
[05]    17.427 ms       100 req         0.026100 ms
[06]    18.364 ms       110 req         0.026052 ms
[07]    19.607 ms       120 req         0.026014 ms
[08]    20.509 ms       130 req         0.025980 ms
[09]    21.654 ms       140 req         0.025951 ms
[10]    22.633 ms       150 req         0.025926 ms

SCAN Results:
 T#  |  Avg Req Time |  Requests   |    Avg Access Time
[00]    12.274 ms       50 req          0.026626 ms
[01]    13.329 ms       60 req          0.026451 ms
[02]    14.429 ms       70 req          0.026325 ms
[03]    15.427 ms       80 req          0.026231 ms
[04]    16.479 ms       90 req          0.026158 ms
[05]    17.548 ms       100 req         0.026100 ms
[06]    18.452 ms       110 req         0.026052 ms
[07]    19.498 ms       120 req         0.026014 ms
[08]    20.603 ms       130 req         0.025980 ms
[09]    21.521 ms       140 req         0.025951 ms
[10]    22.610 ms       150 req         0.025926 ms

LIFO Results:
 T#  |  Avg Req Time |  Requests   |    Avg Access Time
[00]    89.858 ms       50 req          0.026105 ms
[01]    106.133 ms      60 req          0.026017 ms
[02]    124.005 ms      70 req          0.025954 ms
[03]    140.172 ms      80 req          0.025907 ms
[04]    160.687 ms      90 req          0.025871 ms
[05]    175.631 ms      100 req         0.025842 ms
[06]    196.741 ms      110 req         0.025818 ms
[07]    208.732 ms      120 req         0.025798 ms
[08]    229.422 ms      130 req         0.025781 ms
[09]    246.900 ms      140 req         0.025767 ms
[10]    265.401 ms      150 req         0.025755 ms

```
