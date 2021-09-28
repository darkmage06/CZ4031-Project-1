#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

typedef unsigned int uint;
typedef unsigned char uchar;
const int SIZE = 10;

struct Record {
    /*
    * Size of each record = 20 bytes
    */
    char tconst[SIZE];
    float avgRating;
    uint numVotes;
};

struct Address
{
    uchar* blockAddress;
    uint offset;
};

class bufferPool {
private:
    uint bufferPoolSize;
    uint blkSize;
    uint usedBlkSize;
    uint totalRecordSize;
    uint currentBlkSizeUsed;

    uchar* bufferPoolPtr;
    uchar* blkPtr;

    int numBlkAlloc;
    int numBlkAvail;

public:
    bufferPool(uint bufferPoolSize, uint blkSize);

    ~bufferPool();

    Address writeRecordToBlk(uint sizeOfRecord);

    bool blkAvail();

    uint getBufferPoolSize();

    uint getBlkSize();

    uint getUsedBlkSize();

    uint getTotalRecordSize();

    int getNumOfBlkAlloc();

    int getNumOfBlkAvail();
};
#endif
