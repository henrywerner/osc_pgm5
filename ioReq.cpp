class ioReq
{
public:
    ioReq(int t, int s);
    ~ioReq();
    int track, sector;
};

ioReq::ioReq(int t, int s)
{
    this->track = t;
    this->sector = s;
}

ioReq::~ioReq()
{
}
