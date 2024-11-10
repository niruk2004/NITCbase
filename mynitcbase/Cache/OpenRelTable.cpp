#include "OpenRelTable.h"
#include <cstdlib> //u have to add this for malloc to work!
#include <cstring>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];
OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
    tableMetaInfo[i].free=true;
  }

  /************ Setting up Relation Cache entries ************/
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Relation Cache Table****/
  RecBuffer relCatBlockRel(RELCAT_BLOCK);

  Attribute relCatRecordRel[RELCAT_NO_ATTRS];
  relCatBlockRel.getRecord(relCatRecordRel, RELCAT_SLOTNUM_FOR_RELCAT);

  struct RelCacheEntry relCacheEntryRel;
  RelCacheTable::recordToRelCatEntry(relCatRecordRel, &relCacheEntryRel.relCatEntry);
  relCacheEntryRel.recId.block = RELCAT_BLOCK;
  relCacheEntryRel.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;
  relCacheEntryRel.searchIndex = {-1,-1};

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntryRel;

  /**** setting up Attribute Catalog relation in the Relation Cache Table ****/

  // set up the relation cache entry for the attribute catalog similarly
  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT

  RecBuffer relCatBlockAttr(RELCAT_BLOCK);

  Attribute relCatRecordAttr[RELCAT_NO_ATTRS];
  relCatBlockAttr.getRecord(relCatRecordAttr, RELCAT_SLOTNUM_FOR_ATTRCAT);

  struct RelCacheEntry relCacheEntryAttr;
  RelCacheTable::recordToRelCatEntry(relCatRecordAttr, &relCacheEntryAttr.relCatEntry);
  relCacheEntryAttr.recId.block = RELCAT_BLOCK;
  relCacheEntryAttr.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

  //relCacheEntryAttr.searchIndex = {-1,-1};

  // set the value at RelCacheTable::relCache[ATTRCAT_RELID]
  RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = relCacheEntryAttr;



// now to make relation cache for the relation : Students

// RecBuffer relCatBlockStudents(RELCAT_BLOCK);

//   Attribute relCatRecordStudents[RELCAT_NO_ATTRS];
//   relCatBlockStudents.getRecord(relCatRecordStudents, 2);

//   struct RelCacheEntry relCacheEntryStudents;
//   RelCacheTable::recordToRelCatEntry(relCatRecordStudents, &relCacheEntryStudents.relCatEntry);
//   relCacheEntryStudents.recId.block = RELCAT_BLOCK;
//   relCacheEntryStudents.recId.slot = 2;

//   // relCacheEntryStudents.searchIndex = {-1,-1};

//   // set the value at RelCacheTable::relCache[ATTRCAT_RELID]
//   RelCacheTable::relCache[2] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
//   *(RelCacheTable::relCache[2]) = relCacheEntryStudents;






  /************ Setting up Attribute cache entries ************/
  // (we need to populate attribute cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Attribute Cache Table ****/


  // iterate through all the attributes of the relation catalog and create a linked
  // list of AttrCacheEntry (slots 0 to 5)
  // for each of the entries, set
  //    attrCacheEntry.recId.block = ATTRCAT_BLOCK;
  //    attrCacheEntry.recId.slot = i   (0 to 5)
  //    and attrCacheEntry.next appropriately
  // NOTE: allocate each entry dynamically using malloc


  RecBuffer attrCatBlockRel(ATTRCAT_BLOCK);
  Attribute attrCatRecordRel[ATTRCAT_NO_ATTRS];

  struct AttrCacheEntry* attrCacheEntryRel;
  struct AttrCacheEntry* attrCacheEntryRelTemp = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));

  for(int attrCatRelIdx=0;attrCatRelIdx<=5;attrCatRelIdx++){
      attrCatBlockRel.getRecord(attrCatRecordRel,attrCatRelIdx);
      attrCacheEntryRelTemp->next = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
      AttrCacheTable::recordToAttrCatEntry(attrCatRecordRel,&(attrCacheEntryRelTemp->next->attrCatEntry));
      if(attrCatRelIdx==0){
          attrCacheEntryRel = attrCacheEntryRelTemp;
      }
      attrCacheEntryRelTemp->next->recId.block = ATTRCAT_BLOCK;
      attrCacheEntryRelTemp->next->recId.slot = attrCatRelIdx;
      
      attrCacheEntryRelTemp = attrCacheEntryRelTemp->next;
  }
//putting it as null to end the linked list (since it is the last entry of the linked list)
  attrCacheEntryRelTemp->next = nullptr;
//saving in heap
  AttrCacheTable::attrCache[RELCAT_RELID] = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
  *(AttrCacheTable::attrCache[RELCAT_RELID]) = *(attrCacheEntryRel->next);

  attrCacheEntryRelTemp = attrCacheEntryRel;
  attrCacheEntryRel = attrCacheEntryRel->next;
  free(attrCacheEntryRelTemp);



 

  // set the next field in the last entry to nullptr

  //AttrCacheTable::attrCache[RELCAT_RELID] = /* head of the linked list */;

  /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/


  RecBuffer attrCatBlockAttr(ATTRCAT_BLOCK);
  Attribute attrCatRecordAttr[ATTRCAT_NO_ATTRS];

  struct AttrCacheEntry* attrCacheEntryAttr;
  struct AttrCacheEntry* attrCacheEntryAttrTemp = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));

  for(int attrCatAttrIdx=6;attrCatAttrIdx<=11;attrCatAttrIdx++){
      attrCatBlockAttr.getRecord(attrCatRecordAttr,attrCatAttrIdx);
      attrCacheEntryAttrTemp->next = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
      AttrCacheTable::recordToAttrCatEntry(attrCatRecordAttr,&(attrCacheEntryAttrTemp->next->attrCatEntry));
      if(attrCatAttrIdx==6){
          attrCacheEntryAttr = attrCacheEntryAttrTemp;
      }
      attrCacheEntryAttrTemp->next->recId.block = ATTRCAT_BLOCK;
      attrCacheEntryAttrTemp->next->recId.slot = attrCatAttrIdx;
      attrCacheEntryAttrTemp = attrCacheEntryAttrTemp->next;
  }

  attrCacheEntryAttrTemp->next = nullptr;

  AttrCacheTable::attrCache[ATTRCAT_RELID] = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
  *(AttrCacheTable::attrCache[ATTRCAT_RELID]) = *(attrCacheEntryAttr->next);
  attrCacheEntryAttrTemp = attrCacheEntryAttr;
  attrCacheEntryAttr = attrCacheEntryAttr->next;
  free(attrCacheEntryAttrTemp);


  // set up the attributes of the attribute cache similarly.
  // read slots 6-11 from attrCatBlock and initialise recId appropriately

  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID]



  //now to make attribute cache for the relation : Students


  // RecBuffer attrCatBlockStudents(ATTRCAT_BLOCK);
  // Attribute attrCatRecordStudents[ATTRCAT_NO_ATTRS];

  // struct AttrCacheEntry* attrCacheEntryStudents;
  // struct AttrCacheEntry* attrCacheEntryStudentsTemp = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));

  // for(int attrCatStudentsIdx=12;attrCatStudentsIdx<=15;attrCatStudentsIdx++){
  //     attrCatBlockStudents.getRecord(attrCatRecordStudents,attrCatStudentsIdx);
  //     attrCacheEntryStudentsTemp->next = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
  //     AttrCacheTable::recordToAttrCatEntry(attrCatRecordStudents,&(attrCacheEntryStudentsTemp->next->attrCatEntry));
  //     if(attrCatStudentsIdx==12){
  //         attrCacheEntryStudents = attrCacheEntryStudentsTemp;
  //     }
  //     attrCacheEntryStudentsTemp->next->recId.block = ATTRCAT_BLOCK;
  //     attrCacheEntryStudentsTemp->next->recId.slot = attrCatStudentsIdx;
  //     attrCacheEntryStudentsTemp = attrCacheEntryStudentsTemp->next;
  // }

  // attrCacheEntryStudentsTemp->next = nullptr;

  // AttrCacheTable::attrCache[2] = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
  // *(AttrCacheTable::attrCache[2]) = *(attrCacheEntryStudents->next);
  // attrCacheEntryStudentsTemp = attrCacheEntryStudents;
  // attrCacheEntryStudents = attrCacheEntryStudents->next;
  // free(attrCacheEntryStudentsTemp);


  tableMetaInfo[RELCAT_RELID].free = false;
  tableMetaInfo[ATTRCAT_RELID].free = false;
  strcpy(tableMetaInfo[RELCAT_RELID].relName, "RELATIONCAT");
  strcpy(tableMetaInfo[ATTRCAT_RELID].relName, "ATTRIBUTECAT");
}

void freeLinkedList(AttrCacheEntry* head) {
    AttrCacheEntry* current = head;
    AttrCacheEntry* nextNode;

    while (current != nullptr) {
        nextNode = current->next; //Save the next node
        free(current); //Free the current node
        current = nextNode; //Move to the next node
    }

    head = nullptr; // Optional: set head to NULL
}


OpenRelTable::~OpenRelTable() {

  // close all open relations (from rel-id = 2 onwards. Why?) its because relcat and attrcat shud always be open

  for (int i = 2; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free) {
      OpenRelTable::closeRel(i); // we will implement this function later
    }
  }
  // free all the memory that you allocated in the constructor
  
  //relcache free karo

  //releasing the relation cache entry of the attribute catalog
  if(RelCacheTable::relCache[ATTRCAT_RELID]->dirty==true)/* RelCatEntry of the ATTRCAT_RELID-th RelCacheEntry has been modified */{
		RelCatEntry relCatBuffer;
		RelCacheTable::getRelCatEntry(ATTRCAT_RELID,&relCatBuffer);
		Attribute relCatRecord[RELCAT_NO_ATTRS];
		RelCacheTable::relCatEntryToRecord(&relCatBuffer,relCatRecord);
		RecId recId=RelCacheTable::relCache[ATTRCAT_RELID]->recId;
		RecBuffer relCatBlock(recId.block);
		relCatBlock.setRecord(relCatRecord,recId.slot);
	}
	//free(RelCacheTable::relCache[ATTRCAT_RELID]);

  //releasing the relation cache entry of the relation catalog
  if(RelCacheTable::relCache[RELCAT_RELID]->dirty==true)/* RelCatEntry of the RELCAT_RELID-th RelCacheEntry has been modified */{
		RelCatEntry relCatBuffer;
		RelCacheTable::getRelCatEntry(RELCAT_RELID,&relCatBuffer);
		Attribute relCatRecord[ATTRCAT_NO_ATTRS];
		RelCacheTable::relCatEntryToRecord(&relCatBuffer,relCatRecord);
		RecId recId=RelCacheTable::relCache[RELCAT_RELID]->recId;
		RecBuffer attrCatRecord(recId.block);
		attrCatRecord.setRecord(relCatRecord,recId.slot);
	}
	//free(RelCacheTable::relCache[RELCAT_RELID]);


  for(int i=0;i<MAX_OPEN;++i){
    if (RelCacheTable::relCache[i]!=nullptr){
      free(RelCacheTable::relCache[i]);
    }
  }
  //attrcache free karo
  // free the memory allocated for the attribute cache entries of the
  // relation catalog and the attribute catalog

  
  for(int relID=RELCAT_RELID;relID<=ATTRCAT_RELID;relID++){
		AttrCacheEntry *curr=AttrCacheTable::attrCache[relID];
    AttrCacheEntry *next=NULL;
		while(curr!=nullptr){
			next=curr->next;
			if(curr->dirty==true){
				AttrCatEntry attrCatEntry=curr->attrCatEntry;
				Attribute AttrCatrecord[ATTRCAT_NO_ATTRS];
				AttrCacheTable::attrCatEntryToRecord(&attrCatEntry,AttrCatrecord);
				RecBuffer attrCatBlock(curr->recId.block);
				attrCatBlock.setRecord(AttrCatrecord,curr->recId.slot);
			}
			free(curr);
			curr=next;
		}
	}

  for(int i=2;i< MAX_OPEN;++i){
    if (AttrCacheTable::attrCache[i]!=nullptr){
      freeLinkedList(AttrCacheTable::attrCache[i]);
    }
  }

}



/* This function will open a relation having name `relName`.
Since we are currently only working with the relation and attribute catalog, we
will just hardcode it. In subsequent stages, we will loop through all the relations
and open the appropriate one.
*/

int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {
  //hardcoded version
  // if (strcmp(relName,RELCAT_RELNAME)==0){
  //   return RELCAT_RELID;
  // }
  // if (strcmp(relName,ATTRCAT_RELNAME)==0){
  //   return ATTRCAT_RELID;
  // }
  
  // if (strcmp(relName,"Students")==0){
  //   return 2;
  // }
  // if relname is RELCAT_RELNAME, return RELCAT_RELID
  // if relname is ATTRCAT_RELNAME, return ATTRCAT_RELID


  /* traverse through the tableMetaInfo array,
    find the entry in the Open Relation Table corresponding to relName.*/
  for (int i = 0; i < MAX_OPEN; i++) {
    if (strcmp(relName, tableMetaInfo[i].relName) == 0 and tableMetaInfo[i].free == false) {
      return i;
    }
  }
  // if found return the relation id, else indicate that the relation do not
  // have an entry in the Open Relation Table.
  return E_RELNOTOPEN;
}




int OpenRelTable::getFreeOpenRelTableEntry() {

  /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/
  for (int i = 0; i < MAX_OPEN; i++) {
    if (tableMetaInfo[i].free) {
      return i;
    }
  }
  return E_CACHEFULL;

  // if found return the relation id, else return E_CACHEFULL.
}



//my function -

int OpenRelTable::openRel(char relName[ATTR_SIZE]) {
  
  int relId = getRelId(relName);
  /* the relation `relName` already has an entry in the Open Relation Table */
  if(relId>=0){
    // (checked using OpenRelTable::getRelId())
    return relId;
    // return that relation id;
  }

  /* find a free slot in the Open Relation Table
     using OpenRelTable::getFreeOpenRelTableEntry(). */
  // let relId be used to store the free slot.
  relId = OpenRelTable::getFreeOpenRelTableEntry();

  /* free slot not available */
  if (relId==E_CACHEFULL){
    return E_CACHEFULL;
  }

  

  // /****** Setting up Relation Cache entry for the relation ******/

  // /* search for the entry with relation name, relName, in the Relation Catalog using
  //     BlockAccess::linearSearch().
  //     Care should be taken to reset the searchIndex of the relation RELCAT_RELID
  //     before calling linearSearch().*/

  // // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
  // RecId relcatRecId;

  // if (/* relcatRecId == {-1, -1} */) {
  //   // (the relation is not found in the Relation Catalog.)
  //   return E_RELNOTEXIST;
  // }

  // /* read the record entry corresponding to relcatRecId and create a relCacheEntry
  //     on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
  //     update the recId field of this Relation Cache entry to relcatRecId.
  //     use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
  //   NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
  // */

  

  Attribute attrVal; 
  strcpy(attrVal.sVal, relName);
	RelCacheTable::resetSearchIndex(RELCAT_RELID);
    //char* a="RelName";
    char* a =new char[strlen(RELCAT_ATTR_RELNAME) + 1];
      strcpy(a,RELCAT_ATTR_RELNAME);
  	RecId relcatRecId = BlockAccess::linearSearch(RELCAT_RELID,a/* RELCAT_ATTR_RELNAME*/, attrVal, EQ);

	if (relcatRecId.block==-1 and relcatRecId.slot==-1) {
		//! the relation is not found in the Relation Catalog
		return E_RELNOTEXIST;
	}

	// TODO: read the record entry corresponding to relcatRecId and create a relCacheEntry
	// TODO: on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
	RecBuffer relationBuffer (relcatRecId.block);//its the same as relcatrelid
	Attribute relationRecord [RELCAT_NO_ATTRS];
	RelCacheEntry *relCacheBuffer = nullptr;

	relationBuffer.getRecord(relationRecord, relcatRecId.slot);

	//* NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
	relCacheBuffer = (RelCacheEntry*) malloc (sizeof(RelCacheEntry));
	RelCacheTable::recordToRelCatEntry(relationRecord, &(relCacheBuffer->relCatEntry));

	// update the recId field of this Relation Cache entry to relcatRecId.
	relCacheBuffer->recId.block = relcatRecId.block;
	relCacheBuffer->recId.slot = relcatRecId.slot;
	// use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
	RelCacheTable::relCache[relId] = relCacheBuffer;	





  /****** Setting up Attribute Cache entry for the relation ******/

  // let listHead be used to hold the head of the linked list of attrCache entries.
  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  struct AttrCacheEntry* headtemp = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
  struct AttrCacheEntry* head;
  int numberOfAttributes = RelCacheTable::relCache[relId]->relCatEntry.numAttrs;
  

  /*iterate over all the entries in the Attribute Catalog corresponding to each
  attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
  care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
  corresponding to Attribute Catalog before the first call to linearSearch().*/
  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

  for(int attr = 0; attr < numberOfAttributes; attr++)
  {
      /* let attrcatRecId store a valid record id an entry of the relation, relName,
      in the Attribute Catalog.*/
      //char* b= "RelName";
      char* b =new char[strlen(RELCAT_ATTR_RELNAME) + 1];
      strcpy(b,RELCAT_ATTR_RELNAME);
      RecId attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID,b/*RELCAT_ATTR_RELNAME*/, attrVal, EQ);
     
      /* read the record entry corresponding to attrcatRecId and create an
      Attribute Cache entry on it using RecBuffer::getRecord() and
      AttrCacheTable::recordToAttrCatEntry().
      update the recId field of this Attribute Cache entry to attrcatRecId.
      add the Attribute Cache entry to the linked list of listHead .*/

      RecBuffer attrCatBlock(attrcatRecId.block);
		  attrCatBlock.getRecord(attrCatRecord, attrcatRecId.slot);
      headtemp->next = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
      AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&(headtemp->next->attrCatEntry));

      if (attr==0){
        head=headtemp;
      }

      headtemp->next->recId.block = attrcatRecId.block;
		  headtemp->next->recId.slot = attrcatRecId.slot;

		  headtemp = headtemp->next;
      // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
  }
  headtemp->next=nullptr;

  AttrCacheTable::attrCache[relId] = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));


  *(AttrCacheTable::attrCache[relId]) = *(head->next);
  headtemp = head;
  head = head->next;
  free(headtemp);

  // set the relIdth entry of the AttrCacheTable to listHead.

  /****** Setting up metadata in the Open Relation Table for the relation******/

  // update the relIdth entry of the tableMetaInfo with free as false and
  // relName as the input.

  tableMetaInfo[relId].free = false;
	strcpy(tableMetaInfo[relId].relName, relName);


  return relId;
}






int OpenRelTable::closeRel(int relId) {

  /* rel-id corresponds to relation catalog or attribute catalog*/
  if (relId == 0 or relId == 1) {
    return E_NOTPERMITTED;
  }
  /* 0 <= relId < MAX_OPEN */
  if (relId < 0 || relId >= MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  /* rel-id corresponds to a free slot*/
  if (tableMetaInfo[relId].free) {
    return E_RELNOTOPEN;
  }
  // free the memory allocated in the relation and attribute caches which was
  // allocated in the OpenRelTable::openRel() function


  //relcache

  //if it was modified we shud set it in the buffer
  if (RelCacheTable::relCache[relId]->dirty == true) {
		/* Get the Relation Catalog entry from RelCacheTable::relCache
		Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
		Attribute relCatBuffer[RELCAT_NO_ATTRS];
		RelCacheTable::relCatEntryToRecord(&(RelCacheTable::relCache[relId]->relCatEntry), relCatBuffer);

		// declaring an object of RecBuffer class to write back to the buffer
		RecId recId = RelCacheTable::relCache[relId]->recId;
		RecBuffer relCatBlock(recId.block);
  
		// Write back to the buffer using relCatBlock.setRecord() with recId.slot
		relCatBlock.setRecord(relCatBuffer, recId.slot);
	}

  free(RelCacheTable::relCache[relId]);


  
    /****** Releasing the Attribute Cache entry of the relation ******/

  // update `tableMetaInfo` to set `relId` as a free slot
  AttrCacheEntry *head = AttrCacheTable::attrCache[relId];
  AttrCacheEntry *next = head->next;

   // for all the entries in the linked list of the relIdth Attribute Cache entry.


   /*inside the while, if the condition was while(head), it would pose problem as next=next.next 
   would show error if next is null (as null.next is not possible)*/ 
  while (true) {

    // if the entry has been modified:
    if(head->dirty){


      /* Get the Attribute Catalog entry from attrCache
      Then convert it to a record using AttrCacheTable::attrCatEntryToRecord().
      Write back that entry by instantiating RecBuffer class. Use recId
      member field and recBuffer.setRecord() */
      Attribute attrCatRecord [ATTRCAT_NO_ATTRS];
			AttrCacheTable::attrCatEntryToRecord(&(head->attrCatEntry), attrCatRecord);
			RecBuffer attrCatBlockBuffer (head->recId.block);
			attrCatBlockBuffer.setRecord(attrCatRecord, head->recId.slot);


    }


    // free the memory dynamically alloted to this entry in Attribute
    // Cache linked list and assign nullptr to that entry
    free(head);
    head = next;
    if (head==NULL){
      break;
    }
    next = next->next;


  }
  
  free(head);




  /****** Updating metadata in the Open Relation Table of the relation  ******/

  tableMetaInfo[relId].free = true;
  AttrCacheTable::attrCache[relId] = nullptr;
  RelCacheTable::relCache[relId] = nullptr;

  return SUCCESS;
}



