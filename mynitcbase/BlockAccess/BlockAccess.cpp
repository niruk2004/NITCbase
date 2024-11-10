#include "BlockAccess.h"
#include <iostream>
#include <cstring>

//MY LINEAR SEARCH FUNCTION

RecId BlockAccess::linearSearch(int relId,char attrName[ATTR_SIZE], union Attribute attrVal, int op) {
    // get the previous search index of the relation relId from the relation cache
    // (use RelCacheTable::getSearchIndex() function)
    //std::cout << "searching " << attrName << " " << attrVal.sVal << std::endl;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    RecId prevRecId; 
    RelCacheTable::getSearchIndex(relId,&prevRecId);
    // let block and slot denote the record id of the record being currently checked
    int block=-1;
    int slot=-1;
    // if the current search index record is invalid(i.e. both block and slot = -1)
    if (prevRecId.block == -1 && prevRecId.slot == -1)
    {
        // (no hits from previous search; search should start from the
        // first record itself)

        // get the first record block of the relation from the relation cache
        // (use RelCacheTable::getRelCatEntry() function of Cache Layer)
        RelCatEntry firstrecordblock;
        RelCacheTable::getRelCatEntry(relId,&firstrecordblock);
        // block = first record block of the relation
        // slot = 0
        block = firstrecordblock.firstBlk;
        slot = 0;
    }
    else
    {
        // (there is a hit from previous search; search should start from
        // the record next to the search index record)
        block = prevRecId.block;
        slot = prevRecId.slot + 1;
        // block = search index's block
        // slot = search index's slot + 1
    }

    /* The following code searches for the next record in the relation
       that satisfies the given condition
       We start from the record id (block, slot) and iterate over the remaining
       records of the relation
    */
    RelCatEntry relCatBuffer;
	RelCacheTable::getRelCatEntry(relId, &relCatBuffer);
    while (block != -1)
    {
        
        /* create a RecBuffer object for block (use RecBuffer Constructor for
           existing block) */
        RecBuffer blockbuffer(block);
        HeadInfo blockheadinfo;
        
        // get header of the block using RecBuffer::getHeader() function
        blockbuffer.getHeader(&blockheadinfo);
        // get the record with id (block, slot) using RecBuffer::getRecord()
        Attribute theNextRecord[blockheadinfo.numAttrs];
        blockbuffer.getRecord(theNextRecord,slot);
        
        // get slot map of the block using RecBuffer::getSlotMap() function
        unsigned char* slotMapWeUse = (unsigned char *)malloc(sizeof(unsigned char) * blockheadinfo.numSlots);
        blockbuffer.getSlotMap(slotMapWeUse);

        // If slot >= the number of slots per block(i.e. no more slots in this block)
        if (slot>=relCatBuffer.numSlotsPerBlk){
            block = blockheadinfo.rblock;
            slot = 0;
            // update block = right block of block
            // update slot = 0
            continue;  // continue to the beginning of this while loop
        }

        // if slot is free skip the loop
        // (i.e. check if slot'th entry in slot map of block contains SLOT_UNOCCUPIED)
        if (slotMapWeUse[slot]==SLOT_UNOCCUPIED)
        {
            // increment slot and continue to the next record slot
            slot++;
            continue;
        }

        // compare record's attribute value to the the given attrVal as below:
        /*
            firstly get the attribute offset for the attrName attribute
            from the attribute cache entry of the relation using
            AttrCacheTable::getAttrCatEntry()
        */
        AttrCatEntry attrCatBuffer;

		AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatBuffer);
        /* use the attribute offset to get the value of the attribute from
           current record */
        
        
        int cmpVal = compareAttrs(theNextRecord[attrCatBuffer.offset],attrVal,attrCatBuffer.attrType);  // will store the difference between the attributes
        // set cmpVal using compareAttrs()

        /* Next task is to check whether this record satisfies the given condition.
           It is determined based on the output of previous comparison and
           the op value received.
           The following code sets the cond variable if the condition is satisfied.
        */
        if (
            (op == NE && cmpVal != 0) ||    // if op is "not equal to"
            (op == LT && cmpVal < 0) ||     // if op is "less than"
            (op == LE && cmpVal <= 0) ||    // if op is "less than or equal to"
            (op == EQ && cmpVal == 0) ||    // if op is "equal to"
            (op == GT && cmpVal > 0) ||     // if op is "greater than"
            (op == GE && cmpVal >= 0)       // if op is "greater than or equal to"
        ) {
            /*
            set the search index in the relation cache as
            the record id of the record that satisfies the given condition
            (use RelCacheTable::setSearchIndex function)
            */
            RecId newRecId = {block, slot};
			RelCacheTable::setSearchIndex(relId, &newRecId);
            return RecId{block, slot};
        }

        slot++;
    }

    // no record in the relation with Id relid satisfies the given condition
    return RecId{-1, -1};
}






int BlockAccess::renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE]){
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    Attribute newRelationName;    // set newRelationName with newName
    strcpy(newRelationName.sVal, newName);


    // search the relation catalog for an entry with "RelName" = newRelationName
    char* c =new char[strlen(RELCAT_ATTR_RELNAME) + 1];
    strcpy(c,RELCAT_ATTR_RELNAME);
    RecId relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, c, newRelationName, EQ);


    // If relation with name newName already exists (result of linearSearch
    //                                               is not {-1, -1})
    //    return E_RELEXIST;
    if (relcatRecId.block != -1 and relcatRecId.slot !=-1){
        return E_RELEXIST;
    }


    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);


    Attribute oldRelationName;    // set oldRelationName with oldName
    strcpy(oldRelationName.sVal, oldName);


    // search the relation catalog for an entry with "RelName" = oldRelationName
    char* b =new char[strlen(RELCAT_ATTR_RELNAME) + 1];
    strcpy(b,RELCAT_ATTR_RELNAME);
    relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, b, oldRelationName, EQ);
    
    // If relation with name oldName does not exist (result of linearSearch is {-1, -1})
    //    return E_RELNOTEXIST;
    if (relcatRecId.block == -1 and relcatRecId.slot ==-1){
        return E_RELNOTEXIST;
    }

    /* get the relation catalog record of the relation to rename using a RecBuffer
       on the relation catalog [RELCAT_BLOCK] and RecBuffer.getRecord function
    */
    RecBuffer relcatbuffer(relcatRecId.block);
    Attribute relcatrecord[RELCAT_NO_ATTRS];
    relcatbuffer.getRecord(relcatrecord,relcatRecId.slot);
    /* update the relation name attribute in the record with newName.
       (use RELCAT_REL_NAME_INDEX) */
    strcpy(relcatrecord[RELCAT_REL_NAME_INDEX].sVal,newName);
    // set back the record value using RecBuffer.setRecord
    relcatbuffer.setRecord(relcatrecord,relcatRecId.slot);


    /*
    update all the attribute catalog entries in the attribute catalog corresponding
    to the relation with relation name oldName to the relation name newName
    */
    
    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    //for i = 0 to numberOfAttributes :
    //    linearSearch on the attribute catalog for relName = oldRelationName
    //    get the record using RecBuffer.getRecord
    //
    //    update the relName field in the record to newName
    //    set back the record using RecBuffer.setRecord
    
    for (int i=0;i<relcatrecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal;++i){
        char* a =new char[strlen(ATTRCAT_ATTR_RELNAME) + 1];
        strcpy(a,ATTRCAT_ATTR_RELNAME);
        relcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID,a,oldRelationName,EQ);
        RecBuffer attrcatbuffer(relcatRecId.block);
        Attribute attrcatrecord[ATTRCAT_NO_ATTRS];
        attrcatbuffer.getRecord(attrcatrecord, relcatRecId.slot);
        strcpy(attrcatrecord[ATTRCAT_REL_NAME_INDEX].sVal, newName);
        attrcatbuffer.setRecord(attrcatrecord, relcatRecId.slot);
    }
    return SUCCESS;
}


int BlockAccess::renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE]) {

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    Attribute relNameAttr;    // set relNameAttr to relName
    strcpy(relNameAttr.sVal,relName);
    // Search for the relation with name relName in relation catalog using linearSearch()
    // If relation with name relName does not exist (search returns {-1,-1})
    //    return E_RELNOTEXIST;
    char* a =new char[strlen(RELCAT_ATTR_RELNAME) + 1];
    strcpy(a,RELCAT_ATTR_RELNAME);
    RecId relRelId = BlockAccess::linearSearch(RELCAT_RELID,a,relNameAttr,EQ);
    if (relRelId.block==-1 and relRelId.slot==-1){
        return E_RELNOTEXIST;
    }
    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    /* declare variable attrToRenameRecId used to store the attr-cat recId
    of the attribute to rename */
    RecId attrToRenameRecId{-1, -1};
    Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];

    /* iterate over all Attribute Catalog Entry record corresponding to the
       relation to find the required attribute */
    while (true) {
        // linear search on the attribute catalog for RelName = relNameAttr
        char* b =new char[strlen(ATTRCAT_ATTR_RELNAME) + 1];
        strcpy(b,ATTRCAT_ATTR_RELNAME);
        RecId searchindex= BlockAccess::linearSearch(ATTRCAT_RELID,b,relNameAttr,EQ);
        // if there are no more attributes left to check (linearSearch returned {-1,-1})
        //     break;
        if(searchindex.block==-1 and searchindex.slot==-1){
            break;
        }
        /* Get the record from the attribute catalog using RecBuffer.getRecord
          into attrCatEntryRecord */
        RecBuffer attrCatBlock(searchindex.block);
        attrCatBlock.getRecord(attrCatEntryRecord, searchindex.slot);
        // if attrCatEntryRecord.attrName = oldName
        //     attrToRenameRecId = block and slot of this record
        if (strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,oldName)==0){
            attrToRenameRecId.block = searchindex.block;
            attrToRenameRecId.slot = searchindex.slot;
            break;
        }
        // if attrCatEntryRecord.attrName = newName
        //     return E_ATTREXIST;
        if (strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,newName)==0){
            return E_ATTREXIST;
        }
    }

    // if attrToRenameRecId == {-1, -1}
    //     return E_ATTRNOTEXIST;
    if(attrToRenameRecId.block==-1 and attrToRenameRecId.slot==-1){
        return E_ATTRNOTEXIST;
    }

    // Update the entry corresponding to the attribute in the Attribute Catalog Relation.
    /*   declare a RecBuffer for attrToRenameRecId.block and get the record at
         attrToRenameRecId.slot */
    //   update the AttrName of the record with newName
    //   set back the record with RecBuffer.setRecord
    RecBuffer found(attrToRenameRecId.block);
    Attribute attrcatentryfound[ATTRCAT_NO_ATTRS];
    found.getRecord(attrcatentryfound,attrToRenameRecId.slot);
    strcpy(attrcatentryfound[ATTRCAT_ATTR_NAME_INDEX].sVal,newName);
    found.setRecord(attrcatentryfound,attrToRenameRecId.slot);
    return SUCCESS;
}




int BlockAccess::insert(int relId, Attribute *record) {
    // get the relation catalog entry from relation cache
    // ( use RelCacheTable::getRelCatEntry() of Cache Layer)
    RelCatEntry relcatentry;
    RelCacheTable::getRelCatEntry(relId,&relcatentry);
    int blockNum = relcatentry.firstBlk;/* first record block of the relation (from the rel-cat entry)*/

    // rec_id will be used to store where the new record will be inserted
    RecId rec_id = {-1, -1};

    int numOfSlots = relcatentry.numSlotsPerBlk;/* number of slots per record block */
    int numOfAttributes = relcatentry.numAttrs;/* number of attributes of the relation */

    int prevBlockNum = -1;/* block number of the last element in the linked list = -1 */

    /*
        Traversing the linked list of existing record blocks of the relation
        until a free slot is found OR
        until the end of the list is reached
    */
    while (blockNum != -1) {
        // create a RecBuffer object for blockNum (using appropriate constructor!)
        RecBuffer recBuffer(blockNum);
        
        // get header of block(blockNum) using RecBuffer::getHeader() function
        HeadInfo header;
        recBuffer.getHeader(&header);

        // get slot map of block(blockNum) using RecBuffer::getSlotMap() function
        unsigned char slotmap[numOfSlots] ;
        recBuffer.getSlotMap(slotmap);
        // search for free slot in the block 'blockNum' and store it's rec-id in rec_id
        // (Free slot can be found by iterating over the slot map of the block)
        /* slot map stores SLOT_UNOCCUPIED if slot is free and
           SLOT_OCCUPIED if slot is occupied) */

        /* if a free slot is found, set rec_id and discontinue the traversal
           of the linked list of record blocks (break from the loop) */

        /* otherwise, continue to check the next block by updating the
           block numbers as follows:
              update prevBlockNum = blockNum
              update blockNum = header.rblock (next element in the linked
                                               list of record blocks)
        */
        for (int slot = 0; slot < header.numSlots ; slot++) {
          if (slotmap[slot] == SLOT_UNOCCUPIED) {
            rec_id.block = blockNum;
            rec_id.slot = slot;
            break;
          }
        }
        if (rec_id.slot != -1 and rec_id.block != -1) {
         break;
        }

        prevBlockNum = blockNum;
        blockNum = header.rblock;
    }

    //  if no free slot is found in existing record blocks (rec_id = {-1, -1})
    if (rec_id.block==-1 and rec_id.slot==-1){
        // if relation is RELCAT, do not allocate any more blocks
        //     return E_MAXRELATIONS;
        if (relId ==  RELCAT_RELID){
            return E_MAXRELATIONS;
        }
        // Otherwise,
        // get a new record block (using the appropriate RecBuffer constructor!)
        // get the block number of the newly allocated block
        // (use BlockBuffer::getBlockNum() function)
        // let ret be the return value of getBlockNum() function call
        RecBuffer blockBuffer;
        blockNum= blockBuffer.getBlockNum();
        if (blockNum == E_DISKFULL) {
            return E_DISKFULL;
        }

        // Assign rec_id.block = new block number(i.e. ret) and rec_id.slot = 0
        rec_id.block = blockNum;
        rec_id.slot = 0;
        /*
            set the header of the new record block such that it links with
            existing record blocks of the relation
            set the block's header as follows:
            blockType: REC, pblock: -1
            lblock
                  = -1 (if linked list of existing record blocks was empty
                         i.e this is the first insertion into the relation)
                  = prevBlockNum (otherwise),
            rblock: -1, numEntries: 0,
            numSlots: numOfSlots, numAttrs: numOfAttributes
            (use BlockBuffer::setHeader() function)
        */
        HeadInfo blockheader;
        blockheader.pblock = blockheader.rblock = -1;
        blockheader.blockType = REC;
        blockheader.lblock = prevBlockNum;
        blockheader.numAttrs = numOfAttributes;
        blockheader.numEntries = 0;
        blockheader.numSlots = numOfSlots;
        blockBuffer.setHeader(&blockheader);
    
        /*
            set block's slot map with all slots marked as free
            (i.e. store SLOT_UNOCCUPIED for all the entries)
            (use RecBuffer::setSlotMap() function)
        */
        unsigned char slotmap[numOfSlots] ;
        for (int slot = 0; slot < numOfSlots; slot++) {
          slotmap[slot] = SLOT_UNOCCUPIED;
        }
        blockBuffer.setSlotMap(slotmap);
        // if prevBlockNum != -1
        if (prevBlockNum != -1){
            // create a RecBuffer object for prevBlockNum
            // get the header of the block prevBlockNum and
            // update the rblock field of the header to the new block
            // number i.e. rec_id.block
            // (use BlockBuffer::setHeader() function)
            RecBuffer prevblock(prevBlockNum);
            HeadInfo prevheader;
            prevblock.getHeader(&prevheader);
            prevheader.rblock = blockNum;
            prevblock.setHeader(&prevheader);
        }
        // else
        else{
            // update first block field in the relation catalog entry to the
            // new block (using RelCacheTable::setRelCatEntry() function) //this we did after the else block as we have to update the last block anyway
            relcatentry.firstBlk = rec_id.block;
            RelCacheTable::setRelCatEntry(relId, &relcatentry);
            
        }

        // update last block field in the relation catalog entry to the
        // new block (using RelCacheTable::setRelCatEntry() function)
        relcatentry.lastBlk = rec_id.block;
        RelCacheTable::setRelCatEntry(relId, &relcatentry);
    }

    // create a RecBuffer object for rec_id.block
    // insert the record into rec_id'th slot using RecBuffer.setRecord())
    RecBuffer blockbuffer(rec_id.block);
    int ret = blockbuffer.setRecord(record,rec_id.slot);
    if(ret!=SUCCESS){
      printf("Record NOT saved successfully.\n"); //just in case the setrecord aint working properly
    }
    /* update the slot map of the block by marking entry of the slot to
       which record was inserted as occupied) */
    // (ie store SLOT_OCCUPIED in free_slot'th entry of slot map)
    // (use RecBuffer::getSlotMap() and RecBuffer::setSlotMap() functions)
    unsigned char slotmap[numOfSlots];
    blockbuffer.getSlotMap(slotmap);
    slotmap[rec_id.slot]=SLOT_OCCUPIED;
    blockbuffer.setSlotMap(slotmap);
    // increment the numEntries field in the header of the block to
    // which record was inserted
    // (use BlockBuffer::getHeader() and BlockBuffer::setHeader() functions)
    HeadInfo header;
    blockbuffer.getHeader(&header);
    header.numEntries=header.numEntries+1;
    blockbuffer.setHeader(&header);
    // Increment the number of records field in the relation cache entry for
    // the relation. (use RelCacheTable::setRelCatEntry function)
    relcatentry.numRecs++;
    RelCacheTable::setRelCatEntry(relId,&relcatentry);
    //return SUCCESS;


    //did on 11th stage!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    /* B+ Tree Insertions */
    // (the following section is only relevant once indexing has been implemented)

    int flag = SUCCESS;
    // Iterate over all the attributes of the relation
    // (let attrOffset be iterator ranging from 0 to numOfAttributes-1)
    for(int attrOffset =0; attrOffset<numOfAttributes; attrOffset++)
    {
        // get the attribute catalog entry for the attribute from the attribute cache
        // (use AttrCacheTable::getAttrCatEntry() with args relId and attrOffset)
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(relId, attrOffset, &attrCatEntry);

        // get the root block field from the attribute catalog entry

        // if index exists for the attribute(i.e. rootBlock != -1)
        if(attrCatEntry.rootBlock != -1)
        {
            /* insert the new record into the attribute's bplus tree using
             BPlusTree::bPlusInsert()*/
            int retVal = BPlusTree::bPlusInsert(relId, attrCatEntry.attrName,
                                                record[attrOffset], rec_id);

            if (retVal == E_DISKFULL) {
                //(index for this attribute has been destroyed)
                // flag = E_INDEX_BLOCKS_RELEASED
                flag = E_INDEX_BLOCKS_RELEASED;
                BPlusTree::bPlusDestroy(attrCatEntry.rootBlock);
            }
        }
    }

    return flag;
}


/*
NOTE: This function will copy the result of the search to the `record` argument.
      The caller should ensure that space is allocated for `record` array
      based on the number of attributes in the relation.
*/
int BlockAccess::search(int relId, Attribute *record, char attrName[ATTR_SIZE], Attribute attrVal, int op) {
    // Declare a variable called recid to store the searched record
    RecId recId;


    //shit that was before this

    // /* search for the record id (recid) corresponding to the attribute with
    // attribute name attrName, with value attrval and satisfying the condition op
    // using linearSearch() */
    // recId = BlockAccess::linearSearch(relId, attrName, attrVal, op);

    //end of shit



    /* get the attribute catalog entry from the attribute cache corresponding
    to the relation with Id=relid and with attribute_name=attrName  */
    // if this call returns an error, return the appropriate error code
    AttrCatEntry attrcatbuf;
    int ret=AttrCacheTable::getAttrCatEntry(relId,attrName,&attrcatbuf);
	if(ret!=SUCCESS){
		return ret;
	}

    // // get rootBlock from the attribute catalog entry
    // /* if Index does not exist for the attribute (check rootBlock == -1) */ {

    //     /* search for the record id (recid) corresponding to the attribute with
    //        attribute name attrName, with value attrval and satisfying the
    //        condition op using linearSearch()
    //     */
    // }
    if(attrcatbuf.rootBlock==-1){
        recId = BlockAccess::linearSearch(relId, attrName, attrVal, op);
    }

    // /* else */ {
    //     // (index exists for the attribute)

    //     /* search for the record id (recid) correspoding to the attribute with
    //     attribute name attrName and with value attrval and satisfying the
    //     condition op using BPlusTree::bPlusSearch() */
    // }
    else{
        recId = BPlusTree::bPlusSearch(relId,attrName,attrVal,op);
    }






    // if there's no record satisfying the given condition (recId = {-1, -1})
    //    return E_NOTFOUND;
    if (recId.block == -1 and recId.slot == -1){
      return E_NOTFOUND;
    }
    /* Copy the record with record id (recId) to the record buffer (record)
       For this Instantiate a RecBuffer class object using recId and
       call the appropriate method to fetch the record
    */
    RecBuffer recBuffer(recId.block);
    ret = recBuffer.getRecord(record, recId.slot);
    if (ret != SUCCESS){
      return ret;
    }
    return SUCCESS;
}



int BlockAccess::deleteRelation(char relName[ATTR_SIZE]) {
    // if the relation to delete is either Relation Catalog or Attribute Catalog,
    //     return E_NOTPERMITTED
        // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
        // you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)
    if (strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0){
        return E_NOTPERMITTED;
    }
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);


    Attribute relNameAttr; // (stores relName as type union Attribute)
    // assign relNameAttr.sVal = relName
    strcpy((char*)relNameAttr.sVal,(const char*)relName);
    //  linearSearch on the relation catalog for RelName = relNameAttr
    char* a =new char[strlen(RELCAT_ATTR_RELNAME) + 1];
    strcpy(a,RELCAT_ATTR_RELNAME);
    RecId relcatRec_Id = BlockAccess::linearSearch(RELCAT_RELID, a, relNameAttr ,EQ);

    // if the relation does not exist (linearSearch returned {-1, -1})
    //     return E_RELNOTEXIST
    if (relcatRec_Id.block==-1 and relcatRec_Id.slot==-1){
        return E_RELNOTEXIST;
    }


    Attribute relCatEntryRecord[RELCAT_NO_ATTRS];
    /* store the relation catalog record corresponding to the relation in
       relCatEntryRecord using RecBuffer.getRecord */
    RecBuffer relCatBlockBuffer(relcatRec_Id.block);
    relCatBlockBuffer.getRecord(relCatEntryRecord, relcatRec_Id.slot);
    /* get the first record block of the relation (firstBlock) using the
       relation catalog entry record */
    /* get the number of attributes corresponding to the relation (numAttrs)
       using the relation catalog entry record */
    int firstblk = relCatEntryRecord[RELCAT_FIRST_BLOCK_INDEX].nVal;
    int numAttributes = relCatEntryRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal;
    /*
     Delete all the record blocks of the relation
    */
    // for each record block of the relation:
    //     get block header using BlockBuffer.getHeader
    //     get the next block from the header (rblock)
    //     release the block using BlockBuffer.releaseBlock
    //
    //     Hint: to know if we reached the end, check if nextBlock = -1
    int current = firstblk;
    while(current!=-1){
        RecBuffer currentblock(current);
        HeadInfo currentblockhead;
		currentblock.getHeader(&currentblockhead);
        current = currentblockhead.rblock;
        currentblock.releaseBlock();
    }

    /***
        Deleting attribute catalog entries corresponding the relation and index
        blocks corresponding to the relation with relName on its attributes
    ***/

    // reset the searchIndex of the attribute catalog
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    int numberOfAttributesDeleted = 0;

    while(true) {
        RecId attrCatRecId;
        // attrCatRecId = linearSearch on attribute catalog for RelName = relNameAttr
        //A IS RELCAT_ATTR_RELNAME!
        attrCatRecId = BlockAccess::linearSearch(ATTRCAT_RELID,a,relNameAttr,EQ);
        // if no more attributes to iterate over (attrCatRecId == {-1, -1})
        //     break;
        if (attrCatRecId.block ==-1 and attrCatRecId.slot==-1){
            break;
        }

        numberOfAttributesDeleted++;

        // create a RecBuffer for attrCatRecId.block
        // get the header of the block
        // get the record corresponding to attrCatRecId.slot
        RecBuffer attrcatblock (attrCatRecId.block);

		HeadInfo attrcathead;
		attrcatblock.getHeader(&attrcathead);

		Attribute attrCatRecord [ATTRCAT_NO_ATTRS];
		attrcatblock.getRecord(attrCatRecord, attrCatRecId.slot);



        // declare variable rootBlock which will be used to store the root
        // block field from the attribute catalog record.
        int rootBlock = attrCatRecord[ATTRCAT_ROOT_BLOCK_INDEX].nVal; /* get root block from the record */
        // (This will be used later to delete any indexes if it exists)

        // Update the Slotmap for the block by setting the slot as SLOT_UNOCCUPIED
        // Hint: use RecBuffer.getSlotMap and RecBuffer.setSlotMap
        unsigned char slotmap [attrcathead.numSlots];
		attrcatblock.getSlotMap(slotmap);

		slotmap[attrCatRecId.slot] = SLOT_UNOCCUPIED;
		attrcatblock.setSlotMap(slotmap);

        /* Decrement the numEntries in the header of the block corresponding to
           the attribute catalog entry and then set back the header
           using RecBuffer.setHeader */
        attrcathead.numEntries--;

                
		attrcatblock.setHeader(&attrcathead);
        /* If number of entries become 0, releaseBlock is called after fixing
           the linked list.
        */
        if (attrcathead.numEntries==0/*   header.numEntries == 0  */) {
            /* Standard Linked List Delete for a Block
               Get the header of the left block and set it's rblock to this
               block's rblock
        */
            RecBuffer prevBlock (attrcathead.lblock);
			
			HeadInfo leftHeader;
			prevBlock.getHeader(&leftHeader);

			leftHeader.rblock = attrcathead.rblock;
			prevBlock.setHeader(&leftHeader);
            // create a RecBuffer for lblock and call appropriate methods
            
            if (attrcathead.rblock !=INVALID_BLOCKNUM/* header.rblock != -1 */) {
                /* Get the header of the right block and set it's lblock to
                   this block's lblock */
                // create a RecBuffer for rblock and call appropriate methods
                RecBuffer nextBlock(attrcathead.rblock);
				
				HeadInfo rightHeader;
				nextBlock.getHeader(&rightHeader);

				rightHeader.lblock = attrcathead.lblock;
				nextBlock.setHeader(&rightHeader);
            } else {
                // (the block being released is the "Last Block" of the relation.)
                /* update the Relation Catalog entry's LastBlock field for this
                   relation with the block number of the previous block. */
                RelCatEntry relCatEntryBuffer;
				RelCacheTable::getRelCatEntry(ATTRCAT_RELID, &relCatEntryBuffer);
				relCatEntryBuffer.lastBlk = attrcathead.lblock;
            }

            // (Since the attribute catalog will never be empty(why?), we do not
            //  need to handle the case of the linked list becoming empty - i.e
            //  every block of the attribute catalog gets released.)

            // call releaseBlock()
            attrcatblock.releaseBlock();
        }
    


        // (the following part is only relevant once indexing has been implemented)
        // if index exists for the attribute (rootBlock != -1), call bplus destroy
        if (rootBlock != -1) {
            // delete the bplus tree rooted at rootBlock using BPlusTree::bPlusDestroy()
            BPlusTree::bPlusDestroy(rootBlock);
        }
    }



    /*** Delete the entry corresponding to the relation from relation catalog ***/
    // Fetch the header of Relcat block
    HeadInfo relCatHeader;
	relCatBlockBuffer.getHeader(&relCatHeader);

    /* Decrement the numEntries in the header of the block corresponding to the
       relation catalog entry and set it back */
    relCatHeader.numEntries--;
	relCatBlockBuffer.setHeader(&relCatHeader);

    /* Get the slotmap in relation catalog, update it by marking the slot as
       free(SLOT_UNOCCUPIED) and set it back. */
    unsigned char slotmap [relCatHeader.numSlots];
	relCatBlockBuffer.getSlotMap(slotmap);

	slotmap[relcatRec_Id.slot] = SLOT_UNOCCUPIED;
	relCatBlockBuffer.setSlotMap(slotmap);

    /*** Updating the Relation Cache Table ***/
    /** Update relation catalog record entry (number of records in relation
        catalog is decreased by 1) **/
    // Get the entry corresponding to relation catalog from the relation
    // cache and update the number of records and set it back
    // (using RelCacheTable::setRelCatEntry() function)
    RelCatEntry relCatEntryBuffer;
	RelCacheTable::getRelCatEntry(RELCAT_RELID, &relCatEntryBuffer);

	relCatEntryBuffer.numRecs--;
	RelCacheTable::setRelCatEntry(RELCAT_RELID, &relCatEntryBuffer);

    /** Update attribute catalog entry (number of records in attribute catalog
        is decreased by numberOfAttributesDeleted) **/
    // i.e., #Records = #Records - numberOfAttributesDeleted
    
    RelCacheTable::getRelCatEntry(ATTRCAT_RELID, &relCatEntryBuffer);
	relCatEntryBuffer.numRecs -= numberOfAttributesDeleted;
	RelCacheTable::setRelCatEntry(ATTRCAT_RELID, &relCatEntryBuffer);

    // Get the entry corresponding to attribute catalog from the relation
    // cache and update the number of records and set it back
    // (using RelCacheTable::setRelCatEntry() function)

    return SUCCESS;
}




/*
NOTE: the caller is expected to allocate space for the argument `record` based
      on the size of the relation. This function will only copy the result of
      the projection onto the array pointed to by the argument.
*/

int BlockAccess::project(int relId, Attribute *record) {
    // get the previous search index of the relation relId from the relation
    // cache (use RelCacheTable::getSearchIndex() function)
    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId,&prevRecId);
    // declare block and slot which will be used to store the record id of the
    // slot we need to check.
    int block, slot;

    /* if the current search index record is invalid(i.e. = {-1, -1})
       (this only happens when the caller reset the search index)
    */
    if (prevRecId.block == -1 && prevRecId.slot == -1)
    {
        // (new project operation. start from beginning)

        // get the first record block of the relation from the relation cache
        // (use RelCacheTable::getRelCatEntry() function of Cache Layer)
        RelCatEntry firstrecordblock;
        RelCacheTable::getRelCatEntry(relId,&firstrecordblock);
        // block = first record block of the relation
        // slot = 0
        block=firstrecordblock.firstBlk;
        slot=0;
    }
    else
    {
        // (a project/search operation is already in progress)

        // block = previous search index's block
        // slot = previous search index's slot + 1
        block = prevRecId.block;
        slot = prevRecId.slot +1;
    }


    // The following code finds the next record of the relation
    /* Start from the record id (block, slot) and iterate over the remaining
       records of the relation */
    RelCatEntry relcatbuffer;
    RelCacheTable::getRelCatEntry(relId,&relcatbuffer);
    while (block != -1)
    {
        // create a RecBuffer object for block (using appropriate constructor!)
        RecBuffer blockbuffer(block);
        HeadInfo blockheadinfo;
        // get header of the block using RecBuffer::getHeader() function
        blockbuffer.getHeader(&blockheadinfo);
        // get slot map of the block using RecBuffer::getSlotMap() function
        
        unsigned char slotmap[blockheadinfo.numSlots];
        blockbuffer.getSlotMap(slotmap);

        if(slot>=relcatbuffer.numSlotsPerBlk/* slot >= the number of slots per block*/)
        {
            // (no more slots in this block)
            // update block = right block of block
            // update slot = 0
            // (NOTE: if this is the last block, rblock would be -1. this would
            //        set block = -1 and fail the loop condition )
            block = blockheadinfo.rblock;
            slot = 0;
            
        }
        else if (slotmap[slot]==SLOT_UNOCCUPIED/* slot is free */)
        { // (i.e slot-th entry in slotMap contains SLOT_UNOCCUPIED)
            // increment slot
            slot++;
            
        }
        else {
            // (the next occupied slot / record has been found)
            break;
        }
    }

    if (block == -1){
        // (a record was not found. all records exhausted)
        return E_NOTFOUND;
    }

    // declare nextRecId to store the RecId of the record found
    RecId nextRecId{block, slot};

    // set the search index to nextRecId using RelCacheTable::setSearchIndex
    RelCacheTable::setSearchIndex(relId,&nextRecId);
    /* Copy the record with record id (nextRecId) to the record buffer (record)
       For this Instantiate a RecBuffer class object by passing the recId and
       call the appropriate method to fetch the record
    */
    RecBuffer recbuffer(block);
    recbuffer.getRecord(record,slot);
    return SUCCESS;
}



