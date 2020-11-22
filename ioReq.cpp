class ioReq
{
public:
    ioReq();
    ioReq(int t, int s);
    ~ioReq();
    int track, sector;
};

ioReq::ioReq()
{
}

ioReq::ioReq(int t, int s)
{
    this->track = t;
    this->sector = s;
}

ioReq::~ioReq()
{
}
