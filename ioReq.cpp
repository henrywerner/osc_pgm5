/*
file: ioReq.cpp
author: Gherkin
modification history: 
    Gherkin
    November 22nd, 2020
procedures:
    ioReq - Constructor method
    ioReq - Overloaded constructor that defines the request's track and sector values
*/

class ioReq
{
public:
    ioReq();
    ioReq(int t, int s);
    int track, sector;
};

/*
    ioReq()
    author: Gherkin
    date: Nov 22, 2020
    description: Basic constructor method that only exists because I needed to create a temp object
        in my quicksort partition methods.
*/
ioReq::ioReq()
{
}

/*
    ioReq(t, s)
    author: Gherkin
    date: Nov 22, 2020
    description: Overloaded constructor that defines the request's track and sector values
    parameters:
        t  I/P  int  Track value of I/O request
        s  I/P  int  Sector value of I/O request
*/
ioReq::ioReq(int t, int s)
{
    this->track = t;
    this->sector = s;
}
