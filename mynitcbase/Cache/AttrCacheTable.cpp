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
  //which one is bool? nval or sval? 
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