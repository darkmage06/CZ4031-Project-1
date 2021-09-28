#include <iostream>
#include "bufferPool.h"

using namespace std;

bufferPool::bufferPool(uint bufferPoolSize, uint blkSize) {
    this->bufferPoolSize = bufferPoolSize;
    this->blkSize = blkSize;
    this->usedBlkSize = 0;
    this->currentBlkSizeUsed = 0;
    this->totalRecordSize = 0;
    uchar* bufferPoolPtr = nullptr;
    this->bufferPoolPtr = new uchar[bufferPoolSize];
    this->blkPtr = nullptr;
    this->numBlkAlloc = 0;
    this->numBlkAvail = bufferPoolSize / blkSize;
}

// Deallocate the memory and free up the pointer
bufferPool::~bufferPool() {
    delete bufferPoolPtr;
    bufferPoolPtr = nullptr;
}

Address bufferPool::writeRecordToBlk(uint sizeOfRecord) {
    if (sizeOfRecord > blkSize) {
        cout << "Unable to allocate space for record as record size is greater than block size.\n";
        throw "Unable to allocate space for record as record size is greater than block size.\n";
    }
    else if (blkSize < (currentBlkSizeUsed + sizeOfRecord) or numBlkAlloc == 0) {
        if (!blkAvail()) {
            cout << "Unable to allocate space as buffer is full.\n";
            throw "Unable to allocate space as buffer is full.\n";
        }
    }
    totalRecordSize += sizeOfRecord;
    currentBlkSizeUsed += sizeOfRecord;

    Address recordAddress{blkPtr, currentBlkSizeUsed};

    return recordAddress;
}

bool bufferPool::blkAvail() {
    if (numBlkAvail > 0) {
        blkPtr = bufferPoolPtr + (numBlkAlloc * blkSize);
        usedBlkSize += blkSize;
        numBlkAlloc += 1;
        numBlkAvail -= 1;
        currentBlkSizeUsed = 0; // New block assigned with 0 record inside
        return true;
    }
    else
        return false;
}

uint bufferPool::getBufferPoolSize() {
    return bufferPoolSize;
}

uint bufferPool::getBlkSize() {
    return blkSize;
}

uint bufferPool::getUsedBlkSize() {
    return usedBlkSize;
}

uint bufferPool::getTotalRecordSize() {
    return totalRecordSize;
}

int bufferPool::getNumOfBlkAlloc() {
    return numBlkAlloc;
}

int bufferPool::getNumOfBlkAvail() {
    return numBlkAvail;
}