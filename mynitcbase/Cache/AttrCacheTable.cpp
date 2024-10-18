#include "AttrCacheTable.h"

#include <cstring>

AttrCacheEntry* AttrCacheTable::attrCache[MAX_OPEN];

/* returns the attrOffset-th attribute for the relation corresponding to relId
NOTE: this function expects the caller to allocate memory for `*attrCatBuf`
*/
int AttrCacheTable::getAttrCatEntry(int relId, int attrOffset, AttrCatEntry* attrCatBuf) {
  // check if 0 <= relId < MAX_OPEN and return E_OUTOFBOUND otherwise
  if (relId<0 || relId >=MAX_OPEN){
    return E_OUTOFBOUND;
  }

  // check if attrCache[relId] == nullptr and return E_RELNOTOPEN if true
  if (attrCache[relId] == nullptr){
    return E_RELNOTOPEN;
  }
  // traverse the linked list of attribute cache entries
  for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) {
    if (entry->attrCatEntry.offset == attrOffset) {
      
      // copy entry->attrCatEntry to *attrCatBuf and return SUCCESS;
    
      *attrCatBuf = entry->attrCatEntry;
      
      return SUCCESS; 
    }
  }

  // there is no attribute at this offset
  return E_ATTRNOTEXIST;
}

/* Converts a attribute catalog record to AttrCatEntry struct
    We get the record as Attribute[] from the BlockBuffer.getRecord() function.
    This function will convert that to a struct AttrCatEntry type.
*/
void AttrCacheTable::recordToAttrCatEntry(union Attribute record[ATTRCAT_NO_ATTRS],
                                          AttrCatEntry* attrCatEntry) {
  strcpy(attrCatEntry->relName, record[ATTRCAT_REL_NAME_INDEX].sVal);
  // copy the rest of the fields in the record to the attrCacheEntry struct

  strcpy(attrCatEntry->attrName, record[ATTRCAT_ATTR_NAME_INDEX].sVal);
  attrCatEntry->attrType = (int) record[ATTRCAT_ATTR_TYPE_INDEX].nVal;
  attrCatEntry->rootBlock = (int) record[ATTRCAT_ROOT_BLOCK_INDEX].nVal;
  attrCatEntry->offset = (int) record[ATTRCAT_OFFSET_INDEX].nVal;
  attrCatEntry->primaryFlag = (bool) record[ATTRCAT_PRIMARY_FLAG_INDEX].nVal;
}




/* returns the attribute with name `attrName` for the relation corresponding to relId
NOTE: this function expects the caller to allocate memory for `*attrCatBuf`
*/
int AttrCacheTable::getAttrCatEntry(int relId,char attrName[ATTR_SIZE], AttrCatEntry* attrCatBuf) {
  




  // check that relId is valid and corresponds to an open relation
  if (0>relId || relId>=MAX_OPEN){
    return E_OUTOFBOUND;
  }
  if (attrCache[relId] ==  nullptr){
    return E_RELNOTOPEN;
  }
  // iterate over the entries in the attribute cache and set attrCatBuf to the entry that
  //    matches attrName
 
  struct AttrCacheEntry* head = attrCache[relId];
  while (head !=NULL){
    if(strcmp(head->attrCatEntry.attrName,attrName)==0){
      strcpy(attrCatBuf->relName, head->attrCatEntry.relName);
      strcpy(attrCatBuf->attrName, head->attrCatEntry.attrName);

      attrCatBuf->attrType = head->attrCatEntry.attrType;
      attrCatBuf->primaryFlag = head->attrCatEntry.primaryFlag;
      attrCatBuf->rootBlock = head->attrCatEntry.rootBlock;
      attrCatBuf->offset = head->attrCatEntry.offset;
      return SUCCESS;
    }
    head = head->next;
  }

  // no attribute with name attrName for the relation
  return E_ATTRNOTEXIST;
}


//additional
void AttrCacheTable::attrCatEntryToRecord(AttrCatEntry *attrCatEntry, Attribute record[ATTRCAT_NO_ATTRS])
{
    strcpy(record[ATTRCAT_REL_NAME_INDEX].sVal, attrCatEntry->relName);
    strcpy(record[ATTRCAT_ATTR_NAME_INDEX].sVal, attrCatEntry->attrName);

    record[ATTRCAT_ATTR_TYPE_INDEX].nVal = attrCatEntry->attrType;
    record[ATTRCAT_PRIMARY_FLAG_INDEX].nVal = attrCatEntry->primaryFlag;
    record[ATTRCAT_ROOT_BLOCK_INDEX].nVal = attrCatEntry->rootBlock;
    record[ATTRCAT_OFFSET_INDEX].nVal = attrCatEntry->offset;

    // copy the rest of the fields in the record to the attrCacheEntry struct
}


int AttrCacheTable::getSearchIndex(int relId, char attrName[ATTR_SIZE], IndexId *searchIndex) {

  if(relId>=MAX_OPEN or relId<0/*relId is outside the range [0, MAX_OPEN-1]*/) {
    return E_OUTOFBOUND;
  }

  if(attrCache[relId]==nullptr/*entry corresponding to the relId in the Attribute Cache Table is free*/) {
    return E_RELNOTOPEN;
  }

  AttrCacheEntry *attrcacheEntry = AttrCacheTable::attrCache[relId];
  while(attrcacheEntry/* each attribute corresponding to relation with relId */)
  { 
    if (strcmp(attrcacheEntry->attrCatEntry.attrName,attrName)==0/* attrName/offset field of the AttrCatEntry
        is equal to the input attrName/attrOffset */)
    {
      //copy the searchIndex field of the corresponding Attribute Cache entry
      //in the Attribute Cache Table to input searchIndex variable.
      *searchIndex = attrcacheEntry->searchIndex;

      return SUCCESS;
    }
    attrcacheEntry = attrcacheEntry->next;
  }

  return E_ATTRNOTEXIST;
}


int AttrCacheTable::getSearchIndex(int relId, int attrOffset, IndexId *searchIndex) {

  if(relId>=MAX_OPEN or relId<0/*relId is outside the range [0, MAX_OPEN-1]*/) {
    return E_OUTOFBOUND;
  }

  if(attrCache[relId]==nullptr/*entry corresponding to the relId in the Attribute Cache Table is free*/) {
    return E_RELNOTOPEN;
  }

  AttrCacheEntry *attrcacheEntry = AttrCacheTable::attrCache[relId];
  while( attrcacheEntry/* each attribute corresponding to relation with relId */)
  {
    if ( attrcacheEntry->attrCatEntry.offset == attrOffset/* attrName/offset field of the AttrCatEntry
        is equal to the input attrName/attrOffset */)
    {
      //copy the searchIndex field of the corresponding Attribute Cache entry
      //in the Attribute Cache Table to input searchIndex variable.
      *searchIndex = attrcacheEntry->searchIndex;

      return SUCCESS;
    }
    attrcacheEntry = attrcacheEntry->next;
  }

  return E_ATTRNOTEXIST;
}


int AttrCacheTable::setSearchIndex(int relId, char attrName[ATTR_SIZE], IndexId *searchIndex) {

  if(relId>=MAX_OPEN or relId<0/*relId is outside the range [0, MAX_OPEN-1]*/) {
    return E_OUTOFBOUND;
  }

  if(attrCache[relId]==nullptr/*entry corresponding to the relId in the Attribute Cache Table is free*/) {
    return E_RELNOTOPEN;
  }
  
  AttrCacheEntry *attrcacheEntry = AttrCacheTable::attrCache[relId];
  while(attrcacheEntry/* each attribute corresponding to relation with relId */)
  {
    if (strcmp(attrcacheEntry->attrCatEntry.attrName,attrName)==0/* attrName/offset field of the AttrCatEntry
        is equal to the input attrName/attrOffset */)
    { 
      attrcacheEntry->searchIndex = *searchIndex;
      // copy the input searchIndex variable to the searchIndex field of the
      //corresponding Attribute Cache entry in the Attribute Cache Table.

      return SUCCESS;
    }
    attrcacheEntry = attrcacheEntry->next;
  }

  return E_ATTRNOTEXIST;
}


int AttrCacheTable::setSearchIndex(int relId, int attrOffset, IndexId *searchIndex) {

  if(relId>=MAX_OPEN or relId<0/*relId is outside the range [0, MAX_OPEN-1]*/) {
    return E_OUTOFBOUND;
  }

  if(attrCache[relId]==nullptr/*entry corresponding to the relId in the Attribute Cache Table is free*/) {
    return E_RELNOTOPEN;
  }

  AttrCacheEntry *attrcacheEntry = AttrCacheTable::attrCache[relId];
  while(attrcacheEntry/* each attribute corresponding to relation with relId */)
  {
    if (attrcacheEntry->attrCatEntry.offset == attrOffset/* attrName/offset field of the AttrCatEntry
        is equal to the input attrName/attrOffset */)
    { 
      attrcacheEntry->searchIndex = *searchIndex;
      // copy the input searchIndex variable to the searchIndex field of the
      //corresponding Attribute Cache entry in the Attribute Cache Table.

      return SUCCESS;
    }
    attrcacheEntry = attrcacheEntry->next;
  }

  return E_ATTRNOTEXIST;
}

int AttrCacheTable::resetSearchIndex(int relId, char attrName[ATTR_SIZE]) {

  // declare an IndexId having value {-1, -1}
  // set the search index to {-1, -1} using AttrCacheTable::setSearchIndex
  // return the value returned by setSearchIndex
  IndexId indexId = {-1, -1};
  return AttrCacheTable::setSearchIndex(relId, attrName, &indexId);
}

int AttrCacheTable::resetSearchIndex(int relId, int attrOffset) {

  // declare an IndexId having value {-1, -1}
  // set the search index to {-1, -1} using AttrCacheTable::setSearchIndex
  // return the value returned by setSearchIndex
  IndexId indexId = {-1, -1};
  return AttrCacheTable::setSearchIndex(relId, attrOffset, &indexId);
}