#include "StaticBuffer.h"
// the declarations for this class can be found at "StaticBuffer.h"

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];

unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];

StaticBuffer::StaticBuffer() {

  // copy blockAllocMap blocks from disk to buffer (using readblock() of disk)
  // blocks 0 to 3
  for (int i = 0, blockMapslot = 0; i < 4; i++) {
    unsigned char buffer[BLOCK_SIZE];
    Disk::readBlock(buffer, i);
    for (int slot = 0; slot < BLOCK_SIZE; slot++, blockMapslot++) {
      blockAllocMap[blockMapslot] = buffer[slot];
    }
  }

  // initialise all blocks as free
  // set metainfo[bufferindex] with the following values
    //   free = true
    //   dirty = false
    //   timestamp = -1
    //   blockNum = -1
  for (int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;++bufferIndex/*bufferIndex = 0 to BUFFER_CAPACITY-1*/) {
    metainfo[bufferIndex].free = true;
    metainfo[bufferIndex].dirty=false;
    metainfo[bufferIndex].blockNum=-1;
    metainfo[bufferIndex].timeStamp=-1;
  }
}

/*
At this stage, we are not writing back from the buffer to the disk since we are
not modifying the buffer. So, we will define an empty destructor for now. In
subsequent stages, we will implement the write-back functionality here.
*/
StaticBuffer::~StaticBuffer() {

    // copy blockAllocMap blocks from buffer to disk(using writeblock() of disk)
    for (int i = 0, blockMapslot = 0; i < 4; i++) {
      unsigned char buffer[BLOCK_SIZE];
      for (int slot = 0; slot < BLOCK_SIZE; slot++, blockMapslot++) {
        buffer[slot] = blockAllocMap[blockMapslot];
      }
    Disk::writeBlock(buffer, i);
  }

    for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
      if(metainfo[bufferIndex].free==false and metainfo[bufferIndex].dirty==true){
        Disk::writeBlock(blocks[bufferIndex],metainfo[bufferIndex].blockNum);
      }
    }
}

int StaticBuffer::getFreeBuffer(int blockNum) {
  // Check if blockNum is valid (non zero and less than DISK_BLOCKS)
    // and return E_OUTOFBOUND if not valid.
  if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }

  int allocatedBuffer=-1;

// increase the timeStamp in metaInfo of all occupied buffers.
  for( int  i=0;i<BUFFER_CAPACITY;++i){
    if(metainfo[i].free != false){
      metainfo[i].timeStamp+=1;
    }
  }
  int timestamp = 0,replacer =0 ;
  for(int j=0;j<BUFFER_CAPACITY;++j){
    if(metainfo[j].free == true){
      allocatedBuffer = j;
      break;
    }
    if (metainfo[j].timeStamp>timestamp){
      timestamp = metainfo[j].timeStamp;
      replacer = j;
    }
  }
  if (allocatedBuffer == -1){
    if(metainfo[replacer].dirty == true){
      Disk::writeBlock(blocks[replacer], metainfo[replacer].blockNum);
    }
    //we shud make sure that it was done outside because what if it was in cache but was never edited?
    //what if it was just accessed for printing the data or something? then it would not be dirty right?
    allocatedBuffer = replacer;
  }

  metainfo[allocatedBuffer].free = false;
  metainfo[allocatedBuffer].blockNum = blockNum;
  metainfo[allocatedBuffer].dirty = false;
  metainfo[allocatedBuffer].timeStamp = 0;
  
  return allocatedBuffer;
}

/* Get the buffer index where a particular block is stored
   or E_BLOCKNOTINBUFFER otherwise
*/
int StaticBuffer::getBufferNum(int blockNum) {
  // Check if blockNum is valid (between zero and DISK_BLOCKS)
  // and return E_OUTOFBOUND if not valid.
  if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }
  // find and return the bufferIndex which corresponds to blockNum (check metainfo)
  int bufferIndex;
  
  for(bufferIndex=0;bufferIndex<BUFFER_CAPACITY-1;++bufferIndex){
    if (metainfo[bufferIndex].blockNum==blockNum){
        return bufferIndex;
    }
  }
  // if block is not in the buffer
  return E_BLOCKNOTINBUFFER;
}



int StaticBuffer::setDirtyBit(int blockNum){
    // find the buffer index corresponding to the block using getBufferNum().
    int buffernum =  getBufferNum(blockNum);

    // if block is not present in the buffer (bufferNum = E_BLOCKNOTINBUFFER)
    //     return E_BLOCKNOTINBUFFER
    if(buffernum==E_BLOCKNOTINBUFFER){
      return E_BLOCKNOTINBUFFER;
    }
    // if blockNum is out of bound (bufferNum = E_OUTOFBOUND)
    //     return E_OUTOFBOUND
    if (buffernum == E_OUTOFBOUND){
      return E_OUTOFBOUND;
    }
    
    // else
    //     (the bufferNum is valid)
    //     set the dirty bit of that buffer to true in metainfo
    else{
      metainfo[buffernum].dirty=true;
    }
    return SUCCESS;
}



int StaticBuffer::getStaticBlockType(int blockNum){
    // Check if blockNum is valid (non zero and less than number of disk blocks)
    // and return E_OUTOFBOUND if not valid.
    if (blockNum<0 || blockNum>=DISK_BLOCKS){
      return E_OUTOFBOUND;
    }
    // Access the entry in block allocation map corresponding to the blockNum argument
    // and return the block type after type casting to integer.
    return (int)blockAllocMap[blockNum];
}