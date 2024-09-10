#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
// int main(int argc, char *argv[]) {
//   /* Initialize the Run Copy of Disk */
//   Disk disk_run;
//   // StaticBuffer buffer;
//   // OpenRelTable cache;
//   unsigned char buffer[BLOCK_SIZE];
//   Disk::readBlock(buffer,0 );
  
  
//  // Disk::writeBlock(buffer, 7000); 
//  // unsigned char buffer2[BLOCK_SIZE];
//  // char message2[6];
//  // Disk::readBlock(buffer2, 7000);
//  // memcpy(message2, buffer2 + 20, 6);
// for(int i=0;i<50;i++){
//   std::cout <<int( buffer[i]) <<"\n";
// }
//   //return FrontendInterface::handleFrontend(argc, argv);
//   return 0;
// }

void STAGE2_printRelations(){
  RecBuffer relCatBuffer(RELCAT_BLOCK);
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;

  // load the headers of both the blocks into relCatHeader and attrCatHeader.
  // (we will implement these functions later)
  relCatBuffer.getHeader(&relCatHeader);
  attrCatBuffer.getHeader(&attrCatHeader);

  for (int  i = 0 ;i<relCatHeader.numEntries/*i<2*/;++i /*to total relation count*/) {

    Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

    relCatBuffer.getRecord(relCatRecord, i);//i is the slot number of record in relation catalog

    printf("Relation %d: %s\n",i,relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

    int attrCatBufferBlock = ATTRCAT_BLOCK;

    while(attrCatBufferBlock !=-1){

        attrCatBuffer = RecBuffer(attrCatBufferBlock);
        attrCatBuffer.getHeader(&attrCatHeader);

        for (int j = 0; j < attrCatHeader.numEntries; ++j/* j = 0 to number of entries in the attribute catalog */) {

        // declare attrCatRecord and load the attribute catalog entry into it
            Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
            attrCatBuffer.getRecord(attrCatRecord,j);
            
            if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0/* attribute catalog entry corresponds to the current relation */) {
                const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
                printf("%d  %s: %s\n",j, attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal , attrType);
            }

        }
        attrCatBufferBlock = attrCatHeader.rblock;
    }
    printf("\n");
  }
}



void STAGE_2_EXCERCISE2(const char* relname,const char* old_attribute_name,const char* new_attribute_name){
  
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo attrCatHeader;

  attrCatBuffer.getHeader(&attrCatHeader);

  int attrCatBufferBlock = ATTRCAT_BLOCK;

  while(attrCatBufferBlock !=-1){

      attrCatBuffer = RecBuffer(attrCatBufferBlock);
      attrCatBuffer.getHeader(&attrCatHeader);

      for (int j = 0; j < attrCatHeader.numEntries; ++j/* j = 0 to number of entries in the attribute catalog */) {

        // declare attrCatRecord and load the attribute catalog entry into it
          Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
          attrCatBuffer.getRecord(attrCatRecord,j);
            
          if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relname) == 0/* attribute catalog entry corresponds to the relation name which has the attribute that we want to change */) {
              if (strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,old_attribute_name)==0){
                strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,new_attribute_name);
                attrCatBuffer.setRecord(attrCatRecord,j);
                return;
              }
          }

      }
      attrCatBufferBlock = attrCatHeader.rblock;
  }
  printf("\n");
}

void STAGE3_READING_FROM_CACHE(){
    for(int rel_Id= RELCAT_RELID;rel_Id<=2;++rel_Id){
      RelCatEntry relcatbuf;
      RelCacheTable::getRelCatEntry(rel_Id,&relcatbuf);
      printf("Relation: %s\n", relcatbuf.relName);

      for (int j = 0; j<relcatbuf.numAttrs;j++){
          AttrCatEntry attrcatbuf;
          AttrCacheTable::getAttrCatEntry(rel_Id, j,&attrcatbuf);//attribute offset j
          printf("  %s: %d\n", attrcatbuf.attrName, attrcatbuf.attrType);
      }
  
    }
}

void STAGE4(){
  /*IN THIS WE CHANGE THE FRONT END "SELECT FROM TABLE WHERE"
  GET SEARCH INDEX AND SET SEARCH INDEX AND RESET SEARCH INDEX IS ALSO DONE IN THE RELCACHE
  
  in relcacherable we added three functions namely getsearchindex setsearchindex and resetsearchindex.
  the purpose of it is to find the particular searchindex from relcache and assign it to the recid* that
  we want and vice versa. the reset is to assign -1.-1 block and slot number to the relid that we want
  
  
  in attrcachetable, we added a function called getattrcatentry. it alr was added in stage 3 but in 
  stage4 we made another function with same name but different parameter. earlier we searched through
  attrcache with relid and offset. now we search using relid and attribute name. so if it mathces, it is set to
  attcatbuf (it is one of the parameters/destination of the function)
  
  in buffer.cpp file, we make a function called getslotmap. basically what it does is copy the slotmap
  to the parameter slotmap. then we make another function called compareattrs. its purpose is
  An attribute in NITCbase can be either a string or a number. In case the attribute is a number, the 
  operators work as you'd expect. If it is a string, the operation is performed with respect to lexicographic
   order (i.e > would be checked on the first differing letter between two strings). It would be convenient
    in our operation to abstract this implementation detail to a separate function. That is exactly what the
     compareAttrs function in Buffer/BlockBuffer.cpp does. if the attrtype is 1 that is, if it was string,
     we use strcmp and if it was nums we just subtract one from another.
     
     The linearSearch() and project() functions make use of the same search index. Hence, changes in the 
     value of searchIndex will affect the functioning of both these functions.
     
    DOUBT: why did we use attrval in linear search cmprval instead of attrname??
    
    in blockaccess layer we implemented linear search. it takes in relid, attrname, attrval and operation.
    it checks if there is a relation which satisfies the op starting from the last recid.it also sets the recid
    if it gets a hit and resets the recid if it doesnt get a hit at all
    
    then we go to the algebra layer and implement a function called select
    used to select all the records that satisfy a condition.
    the arguments of the function are
    - srcRel - the source relation we want to select from
    - targetRel - the relation we want to select into. (ignore for now)
    - attr - the attribute that the condition is checking
    - op - the operator of the condition
    - strVal - the value that we want to compare against (represented as a string)
    
    
    
    In openreltable we added a function called getrelid which checks for the relname which is given as parameter
    with the relcat relname and attrcat relname and returns the rel id if it matches to any of it. else it returns
    that the rel is not open!

    */
}

void STAGE5(){
  /*
  
  first we go to frontend.cpp to  the open table and close table commands to connect to the schema layer.
  
  in the schema layer we connect it to the openrel table so that it calls the openrel and closerel properly.
  also in the closerel, it checks if it is trying to close attrcat or relcat to ensure accidentally closing them
  it sends the notpermitted error!

  while doing the openrel table, there was a datatype mismatch in getrelid function!
  apart from that in openreltable, we added the metainfo to it as well as added the free in the ~openreltable
  getFreeOpenRelTableEntry - returns Returns index of an unoccupied entry in the Open Relation Table by going thru metinfo
  getrelid - same as bfr except it checks the metainfo now
  openrel -  it had the same datatype mismatch as getrelid. Creates an entry for the input relation in the Open Relation
              Table and returns the corresponding relation id.
          
  closerel -freeing the memeory created during open rel
  added a,b,c,d,e,f,g,h,i,j,k relations as part of the excercise
  also trying to call the linearsearch with const attrname will result in warnings so had to use char* and allocate length 
  and then use strcpy to cpy the const char* to char* and then use the cahr* as the parameter!
  
  */
}

void STAGE6(){
  /*
  OBJECTIVE:
  Implement the commands to rename relations and attributes
  Implement the LRU algorithm to free up space in the buffer by writing back to the disk when the buffer becomes fully occupied


  DONT FORGET TO ADD IN NOTE THAT WE HAD TO STRCPY THE OLDNAME TO OLDRELNAME SO THAT IT BECOMES AN ATTRIBUTE TYPE!! 
  (IN RENAMEREL IN BLCOKACCESS)

  - in static buffer , why did we update all the timestamps by 1 in the getfreebuffer func when it was not done in the git??
  i think its because since we are just replacing the most used one the relative order remains the same so its fine either way.

  - also abt the replacer being assinged to the bufferindex if it was not dirty in the first place. why is it like that in the git?
  
  in xfs when we try to make a relation named Books it will get fked up first attribute is ignored.
  */
}

void tips(){
  /*
  Disk::readblock is used at 3 places - blockbuffer (for loadblockandgetbufferptr if it wasnt in buffer so we have to load into 
                  buffer for that we use read) , staticbuffer(for copying block map from diskin the staticbuffer costructor and
                  in the disk.cpp for defining)
  

  Disk::writeblock is used at 4 places - 2 times in destructor of staticbuffer (one to writeback blockmap and other if its dirty),
                  and one time at getfreebuffer (check if dirty while replacing and do it if true) and then in disk.cpp for defining
                  the writeblock
          
  linearsearch  in select for printing, rename relation ,  rename attribute,  search in blockaccess, deleterelation, openreltable 
                openrel function and in schema for createrel
  */
}
int main(int argc, char *argv[]) {
  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;
  // create objects for the relation catalog and attribute catalog
    //stage 2
        //STAGE_2_EXCERCISE2("Students","Class","Batch");
        //STAGE2_printRelations();
        //STAGE2_printRelations();
        //STAGE3_READING_FROM_CACHE();
  return FrontendInterface::handleFrontend(argc, argv);
  //return 0;
}
