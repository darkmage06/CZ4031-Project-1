#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

typedef unsigned int uint;
typedef unsigned char uchar;
const int SIZE = 11;

struct Record 
{
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

struct LLNode
{
    Address address;
    int size;
    struct LLNode* next;
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

    void deleteRecord(Address address);

    uint getBufferPoolSize();

    uint getBlkSize();

    uint getUsedBlkSize();

    uint getTotalRecordSize();

    int getNumOfBlkAlloc();

    int getNumOfBlkAvail();
};
#endif
