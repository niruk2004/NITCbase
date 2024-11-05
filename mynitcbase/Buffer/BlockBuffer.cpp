#include "BlockBuffer.h"
#include <iostream>
#include <cstdlib>
#include <cstring>

// the declarations for these functions can be found in "BlockBuffer.h"

BlockBuffer::BlockBuffer(int blockNum) {
  // initialise this.blockNum with the argument
  this->blockNum = blockNum;
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}
RecBuffer::RecBuffer() : BlockBuffer('R'){}

//stage 10 index
IndBuffer::IndBuffer(char blockType) : BlockBuffer(blockType) {}
IndBuffer::IndBuffer(int blockNum) : BlockBuffer(blockNum) {}

IndInternal::IndInternal() : IndBuffer('I') {}
IndInternal::IndInternal(int blockNum) : IndBuffer(blockNum) {}

IndLeaf::IndLeaf() : IndBuffer('L'){} // this is the way to call parent non-default constructor.'L' used to denote IndLeaf.
//this is the way to call parent non-default constructor.
IndLeaf::IndLeaf(int blockNum) : IndBuffer(blockNum){}
//stage 10 index thing over rest made functions are at last

// load the block header into the argument pointer
int BlockBuffer::getHeader(struct HeadInfo *head) {

  unsigned char* bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);

  if (ret != SUCCESS) {
    return ret;   // return any errors that might have occured in the process
  }

  // read the block at this.blockNum into the buffer

  //Disk::readBlock(bufferPtr, blockNum); this is not needed anymore as we are using loadblockandgetbuffer

  // populate the numEntries, numAttrs and numSlots fields in *head
  memcpy(&head->blockType, bufferPtr, 4); //the offsets are provided correcly according to the blockbuffer.h
  memcpy(&head->pblock, bufferPtr+4, 4);
  memcpy(&head->lblock, bufferPtr+8, 4);
  memcpy(&head->rblock, bufferPtr+12, 4);
  memcpy(&head->numEntries, bufferPtr+16, 4);
  memcpy(&head->numAttrs, bufferPtr+20, 4);
  memcpy(&head->numSlots, bufferPtr+24, 4);
  memcpy(&head->reserved, bufferPtr+28, 4);


  return SUCCESS;
}



// load the record at slotNum into the argument pointer
int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
  
  struct HeadInfo head;

  // get the header using this.getHeader() function
  this->getHeader(&head);

  int currentBlockNum = blockNum;

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;

  // read the block at this.blockNum into a buffer
  unsigned char* bufferPtr;

  //Disk::readBlock(buffer,currentBlockNum);
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
     - each record will have size attrCount * ATTR_SIZE
     - slotMap will be of size slotCount
  */
  int recordSize = attrCount * ATTR_SIZE;

  unsigned char *slotPointer = bufferPtr + HEADER_SIZE + slotCount + (recordSize * slotNum);

  // load the record into the rec data structure
  memcpy(rec, slotPointer, recordSize);

  return SUCCESS;
}

int RecBuffer::setRecord(union Attribute *rec, int slotNum) {
  
  struct HeadInfo head;

  // get the header using this.getHeader() function
  this->getHeader(&head);

  int currentBlockNum = blockNum;

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;

  // read the block at this.blockNum into a buffer
  unsigned char* bufferPtr;

  //Disk::readBlock(buffer,currentBlockNum); as we are using loadblockandgetbuffer we dont need this readblock anymore
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }
  // if input slotNum is not in the permitted range return E_OUTOFBOUND.
  if(slotNum>slotCount or slotNum<0){
      return E_OUTOFBOUND;
  }
  /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
     - each record will have size attrCount * ATTR_SIZE
     - slotMap will be of size slotCount
  */
  int recordSize = attrCount * ATTR_SIZE;

  unsigned char *slotPointer = bufferPtr + HEADER_SIZE + slotCount + (recordSize * slotNum);

  // load the record into the rec data structure
  memcpy( slotPointer,rec, recordSize);
  //Disk::writeBlock(bufferPtr,blockNum); now we dont need this as it does the write backs when replacing in cache
  ret = StaticBuffer::setDirtyBit(this->blockNum);
  if(ret!=SUCCESS){
      printf("something wrong with setdirtyfunction!!");
  }//this is acutally not necessary as long as the dirty function works fine. this can be deleted later.
  return SUCCESS;
}


int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr) {
  // check whether the block is already present in the buffer using StaticBuffer.getBufferNum()
  int bufferNum = StaticBuffer::getBufferNum(this->blockNum);
  if (bufferNum !=E_BLOCKNOTINBUFFER){
    for(int i=0;i<BUFFER_CAPACITY;++i){
      StaticBuffer::metainfo[i].timeStamp+=1; //this actually doesnt matter coz we are updating the buffernum's timestamp to 0
    }
    StaticBuffer::metainfo[bufferNum].timeStamp = 0;
  }
  else{
    bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
    if (bufferNum == E_OUTOFBOUND) {
      return E_OUTOFBOUND;
    }
    Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
  }

  // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr
  *buffPtr = StaticBuffer::blocks[bufferNum];

  return SUCCESS;
}



/* used to get the slotmap from a record block
NOTE: this function expects the caller to allocate memory for `*slotMap`
*/
int RecBuffer::getSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;

  // get the starting address of the buffer containing the block using loadBlockAndGetBufferPtr().
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  struct HeadInfo head;
  // get the header of the block using getHeader() function
  RecBuffer currentblock(this->blockNum); 
  currentblock.getHeader(&head);

  int slotCount = head.numSlots /* number of slots in block from header */;

  // get a pointer to the beginning of the slotmap in memory by offsetting HEADER_SIZE
  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;

  // copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)
  memcpy(slotMap,slotMapInBuffer,slotCount);

  return SUCCESS;
}



int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {

    double diff;
    if (attrType == 1){
         diff = strcmp(attr1.sVal, attr2.sVal);
    }

    else{
      diff = (attr1.nVal - attr2.nVal) ;
    }

    
    if (diff > 0) return 1;
    else if (diff < 0) return -1;
    else return 0;
    
}


int BlockBuffer::setHeader(struct HeadInfo *head){

    unsigned char *bufferPtr;
    // get the starting address of the buffer containing the block using
    // loadBlockAndGetBufferPtr(&bufferPtr).
    int ret= loadBlockAndGetBufferPtr(&bufferPtr);


    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    if (ret!= SUCCESS){
      return ret;
    }
    // cast bufferPtr to type HeadInfo*
    struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;

    // copy the fields of the HeadInfo pointed to by head (except reserved) to
    // the header of the block (pointed to by bufferHeader)
    //(hint: bufferHeader->numSlots = head->numSlots )
    bufferHeader->numSlots = head->numSlots;
    bufferHeader->lblock = head->lblock;
    bufferHeader->numEntries = head->numEntries;
    bufferHeader->pblock = head->pblock;
    bufferHeader->rblock = head->rblock;
    bufferHeader->blockType = head->blockType;
    bufferHeader->numAttrs=head->numAttrs;
    // update dirty bit by calling StaticBuffer::setDirtyBit()
    // if setDirtyBit() failed, return the error code
    int setDirty = StaticBuffer::setDirtyBit(this->blockNum);
    return setDirty;

    //return SUCCESS;
}


int BlockBuffer::setBlockType(int blockType){

    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret= loadBlockAndGetBufferPtr(&bufferPtr);
    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    if (ret!= SUCCESS){
      return ret;
    }
    // store the input block type in the first 4 bytes of the buffer.
    // (hint: cast bufferPtr to int32_t* and then assign it)
    // *((int32_t *)bufferPtr) = blockType;
    *((int32_t *)bufferPtr) = blockType;

    // update the StaticBuffer::blockAllocMap entry corresponding to the
    // object's block number to `blockType`.
    StaticBuffer::blockAllocMap[this->blockNum] = blockType;

    // update dirty bit by calling StaticBuffer::setDirtyBit()
    // if setDirtyBit() failed
        // return the returned value from the call
    ret = StaticBuffer::setDirtyBit(this->blockNum);
    return ret;
    // return SUCCESS
}


int BlockBuffer::getFreeBlock(int blockType){

    // iterate through the StaticBuffer::blockAllocMap and find the block number
    // of a free block in the disk.
    int blocknum;
    for(blocknum=0;blocknum<DISK_BLOCKS;blocknum++){
      if (StaticBuffer::blockAllocMap[blocknum] == UNUSED_BLK){
        break;
      }

    }
    // if no block is free, return E_DISKFULL.
    if (blocknum == DISK_BLOCKS)
    return E_DISKFULL;

    // set the object's blockNum to the block number of the free block.
    this->blockNum = blocknum;

    // find a free buffer using StaticBuffer::getFreeBuffer() .
    int bufferNum = StaticBuffer::getFreeBuffer(blockNum);
    
    // initialize the header of the block passing a struct HeadInfo with values
    // pblock: -1, lblock: -1, rblock: -1, numEntries: 0, numAttrs: 0, numSlots: 0
    // to the setHeader() function.
    struct HeadInfo header;
    header.pblock=-1;
    header.lblock=-1;
    header.rblock=-1;
    header.numEntries=0;
    header.numSlots=0;
    header.numAttrs=0;
    header.numSlots=0;

    setHeader(&header);

    // update the block type of the block to the input block type using setBlockType().
    setBlockType(blockType);
    // return block number of the free block.
    return blocknum;
}


BlockBuffer::BlockBuffer(char blockType){
    // allocate a block on the disk and a buffer in memory to hold the new block of
    // given type using getFreeBlock function and get the return error codes if any.
    int blocktype = blockType == 'R' ? REC : UNUSED_BLK;
    int blocknum = getFreeBlock(blocktype);
    // set the blockNum field of the object to that of the allocated block
    // number if the method returned a valid block number,
    // otherwise set the error code returned as the block number.
    if (blocknum<0 || blocknum>=DISK_BLOCKS){
      std::cout << "Error: assigned block is not in range\n";
      this->blockNum=blocknum;
      return;
    }
    this->blockNum = blockNum;
    // (The caller must check if the constructor allocatted block successfully
    // by checking the value of block number field.)
}


int RecBuffer::setSlotMap(unsigned char *slotMap) {
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block using
       loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    if (ret!=SUCCESS){
      return ret;
    }

    // get the header of the block using the getHeader() function
    HeadInfo head;
    getHeader(&head);
    int numSlots = head.numSlots /* the number of slots in the block */;

    // the slotmap starts at bufferPtr + HEADER_SIZE. Copy the contents of the
    // argument `slotMap` to the buffer replacing the existing slotmap.
    // Note that size of slotmap is `numSlots`
    memcpy(bufferPtr + HEADER_SIZE, slotMap, numSlots);
    // update dirty bit using StaticBuffer::setDirtyBit
    // if setDirtyBit failed, return the value returned by the call
    ret = StaticBuffer::setDirtyBit(this->blockNum);
    
    return ret;
}


int BlockBuffer::getBlockNum(){
    //return corresponding block number.
    return this->blockNum;
}



void BlockBuffer::releaseBlock(){

    // if blockNum is INVALID_BLOCKNUM (-1), or it is invalidated already, do nothing
    if(blockNum == INVALID_BLOCKNUM || StaticBuffer::blockAllocMap[blockNum] == UNUSED_BLK){
      //it is invalid!!!
      printf("invalid block number! check releaseblock in blockbuffer");
      return;
    }
    
    // else
        /* get the buffer number of the buffer assigned to the block
           using StaticBuffer::getBufferNum().
           (this function return E_BLOCKNOTINBUFFER if the block is not
           currently loaded in the buffer)
            */

        // if the block is present in the buffer, free the buffer
        // by setting the free flag of its StaticBuffer::tableMetaInfo entry
        // to true.

        
    int buffernumber = StaticBuffer::getBufferNum(blockNum);
    if (buffernumber < 0 || buffernumber >= BUFFER_CAPACITY){
      printf("block is not loaded in buffer !!");
      return;
    }

    StaticBuffer::metainfo[buffernumber].free = true;
    
    
    // free the block in disk by setting the data type of the entry
    // corresponding to the block number in StaticBuffer::blockAllocMap
    // to UNUSED_BLK.
    StaticBuffer::blockAllocMap[blockNum] = UNUSED_BLK;

    // set the object's blockNum to INVALID_BLOCK (-1)
    this->blockNum = INVALID_BLOCKNUM;
}



int IndInternal::getEntry(void *ptr, int indexNum) {

    // if the indexNum is not in the valid range of [0, MAX_KEYS_INTERNAL-1]
    //     return E_OUTOFBOUND.
    if (indexNum < 0 or indexNum>=MAX_KEYS_INTERNAL){
      return E_OUTOFBOUND;
    }

    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
    //     return the value returned by the call.
    if (ret!=SUCCESS){
      return ret;
    }

    // typecast the void pointer to an internal entry pointer
    struct InternalEntry *internalEntry = (struct InternalEntry *)ptr;

    /*
    - copy the entries from the indexNum`th entry to *internalEntry
    - make sure that each field is copied individually as in the following code
    - the lChild and rChild fields of InternalEntry are of type int32_t
    - int32_t is a type of int that is guaranteed to be 4 bytes across every
      C++ implementation. sizeof(int32_t) = 4
    */

    /* the indexNum'th entry will begin at an offset of
       HEADER_SIZE + (indexNum * (sizeof(int) + ATTR_SIZE) )         [why?]
       from bufferPtr */
    unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * 20);

    memcpy(&(internalEntry->lChild), entryPtr, sizeof(int32_t));
    memcpy(&(internalEntry->attrVal), entryPtr + 4, sizeof(Attribute));
    memcpy(&(internalEntry->rChild), entryPtr + 20, 4);

    return SUCCESS;
}

int IndLeaf::getEntry(void *ptr, int indexNum) {

    // if the indexNum is not in the valid range of [0, MAX_KEYS_LEAF-1]
    //     return E_OUTOFBOUND.
    if (indexNum < 0 or indexNum>=MAX_KEYS_LEAF){
      return E_OUTOFBOUND;
    }
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
    //     return the value returned by the call.
    if (ret!=SUCCESS){
      return ret;
    }
    // copy the indexNum'th Index entry in buffer to memory ptr using memcpy

    /* the indexNum'th entry will begin at an offset of
       HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE)  from bufferPtr */
    unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE);
    memcpy((struct Index *)ptr, entryPtr, LEAF_ENTRY_SIZE);

    return SUCCESS;
}

int IndInternal::setEntry(void *ptr, int indexNum) {
  return 0;
}

int IndLeaf::setEntry(void *ptr, int indexNum) {
  return 0;
}