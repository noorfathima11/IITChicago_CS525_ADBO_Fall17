#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<math.h>

#include "storage_mgr.h"

FILE *fp;

extern void initStorageManager (void) {
	
	fp = NULL;
}

RC createPageFile (char *fileName){
     
    //my runs

	//Req 1: Create a new page file
	//Step 1: Declare a file pointer variable
	FILE *fp ; //fp is a file pointer variable that is declared as a pointer to a structure of type FILE 
               //Here we have declared it locally 

	//Step 2: Open a file 
	fp = fopen(fileName, "w" ); 

	//Step 3: Lets check if file has been successfully opened
	if(fp == NULL)
	{
      printf("Error in opening the file\n");
      exit(0);
	}

    //Req 2: Initial file size should be one page only. SM is dealing with Pages (blocks) of fixed size (PAGE_SIZE)
    //  ___________  
    // |___________|  -> One block/page
    // PAGE_SIZE:4096
    
    //Step 1: We declare a pointer variable 
    char *indexPage, *initialPage;


    //Step 2: We need only one block/page of allocation. 
     indexPage = (char *) calloc (PAGE_SIZE, sizeof(char));
     initialPage = (char *) calloc (PAGE_SIZE, sizeof(char)); // PAGE_SIZE is hardcoded to 4096, so our initialPage size is 4096 bytes (because sizeof(char) =1).

    
    //Step 3: Lets check if the allocation is successful. If it is successful it should return the address of first byte of allocated memory.
    //If specified size of memory is not available then "overflow of memory" will happen, in that case our function will return NULL
    if (initialPage == NULL || indexPage == NULL)
    {
      printf("Insufficient memory\n" );
      return;
    }

     strcat(indexPage,"1\n");

    //Req 3: Should fill this single block/page with '\0' bytes. i.e this block/page should be made sure is empty
    fwrite(initialPage, sizeof(char), PAGE_SIZE, fp); // The data from the buffer which is pointed by initialPage is written into the file associated with fp.
    printf("Tagging initial page to file successful\n");

    //Now lets deallocate the allocated block of memory which we did using calloc. 
      free(initialPage);
    
    //Now lets close our open file pointer fp. It also flushes all the buffers. 
      fclose(fp);

    //Now lets return a RC value which will indicate if our operation was successful
      return RC_OK; 
       

}
extern RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
	
	fp = fopen(fileName, "r");

	
	if(fp != NULL) {

		struct stat detail;

		fHandle->fileName = fileName;
		fHandle->curPagePos = 0;

		
		if(fstat(fileno(fp), &detail) < 0)    
			return RC_ERROR;
		fHandle->totalNumPages = detail.st_size/ PAGE_SIZE;	
	}

	else 
	{ 

		return RC_FILE_NOT_FOUND;	
	}
	fclose(fp);
	return RC_OK;
}


RC closePageFile (SM_FileHandle *fHandle)
{

  //my runs
  int closeFileDescriptor = fclose(fHandle->mgmtInfo); 

  if(!closeFileDescriptor)
  {
    return RC_OK;
  }

  return RC_FILE_NOT_FOUND;
}

RC destroyPageFile (char *fileName)
{
	// my runs
	int isDestroy = remove(fileName);
  
  if (!isDestroy)
    return RC_OK;

  return RC_FILE_NOT_FOUND;

}

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	
	if (pageNum > fHandle->totalNumPages || pageNum < 0)
        	return RC_READ_NON_EXISTING_PAGE;

		
	fp = fopen(fHandle->fileName, "r");

	
	if(fp != NULL)
	{
		int seeking = fseek(fp, (pageNum * PAGE_SIZE), SEEK_SET);
	    if(seeking != 0) 
	    {
	    	return RC_READ_NON_EXISTING_PAGE; 
	    } 
	    else 
	    {
		    if(fread(memPage, sizeof(char), PAGE_SIZE, fp) < PAGE_SIZE)
			   return RC_ERROR;
	    }
		
	}
	else
	{
		return RC_FILE_NOT_FOUND;
	}
	
	
	fHandle->curPagePos = ftell(fp); 
	    	
	fclose(fp);
	
    	return RC_OK;
}

extern int getBlockPos (SM_FileHandle *fHandle) {
	
	return fHandle->curPagePos;
}

extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	
	fp = fopen(fHandle->fileName, "r");
	
	if(fp == NULL)
	{
	   int i;
	   for(i = 0; i < PAGE_SIZE; i++) 
	   {
		  
		char c = fgetc(fp);
		if(feof(fp))
			break;
		else
			memPage[i] = c;
	   }
	}
    else
    {
    	return RC_FILE_NOT_FOUND;
    }

	fHandle->curPagePos = ftell(fp); 

	fclose(fp);
	return RC_OK;
}

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {

	if(fHandle->curPagePos >= PAGE_SIZE)
	{	
		int thisBlock = fHandle->curPagePos / PAGE_SIZE;
		int begiPosi = (PAGE_SIZE * (thisBlock - 2));

	    fp = fopen(fHandle->fileName, "r");
	
		if(fp == NULL)
		{
		   int i;
	       fseek(fp, begiPosi, SEEK_SET);
		   for(i = 0; i < PAGE_SIZE; i++) 
		   {
			  memPage[i] = fgetc(fp);
		   }

		
		   fHandle->curPagePos = ftell(fp); 

   		   fclose(fp);
		   return RC_OK;
	    }
		
		else
		{
			return RC_FILE_NOT_FOUND;
		}
	}
	else
	{
		printf("\n First block: Previous block not present.");
		return RC_READ_NON_EXISTING_PAGE;	
	}
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
		
	int thisBlock = fHandle->curPagePos / PAGE_SIZE;
	int begiPosi = (PAGE_SIZE * (thisBlock - 2));
	
	
	fp = fopen(fHandle->fileName, "r");

	
	if(fp != NULL)
	{
	
	    fseek(fp, begiPosi, SEEK_SET);
	
	    int i;

	    for(i = 0; i < PAGE_SIZE; i++) 
	    {
		   char c = fgetc(fp);		
		   if(feof(fp))
			  break;
		   memPage[i] = c;
	    }
	
	    fHandle->curPagePos = ftell(fp);

    }

	else
	{
      return RC_FILE_NOT_FOUND;
	}

	
	fclose(fp);
	return RC_OK;		
}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	
	if(fHandle->curPagePos != PAGE_SIZE) 
	{
		fp = fopen(fHandle->fileName, "r");
		
		int thisBlock = fHandle->curPagePos / PAGE_SIZE;
		int begiPosi = (PAGE_SIZE * (thisBlock - 2));
	
		if(fp == NULL)
		{
		   int i;
		   fseek(fp, begiPosi, SEEK_SET);
		
		   for(i = 0; i < PAGE_SIZE; i++) 
		   {
			 char c = fgetc(fp);		
			 if(feof(fp))
				break;
			 memPage[i] = c;
		   }
	    }
	    else
	    {
	    	return RC_FILE_NOT_FOUND;
	    }

		
		fHandle->curPagePos = ftell(fp); 

		
		fclose(fp);
		return RC_OK;
	}

	else
	{
		printf("\n Last block: Next block not present.");
		return RC_READ_NON_EXISTING_PAGE;
	}
}

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	
	fp = fopen(fHandle->fileName, "r");

	if(fp != NULL)
	{
	  int i;
	  int begiPosi = (fHandle->totalNumPages - 1) * PAGE_SIZE;
      fseek(fp, begiPosi, SEEK_SET);
	
	  for(i = 0; i < PAGE_SIZE; i++) 
	  {
		char c = fgetc(fp);		
		if(feof(fp))
			break;
		memPage[i] = c;
	}

    }
    else
    {
    	return RC_FILE_NOT_FOUND;
    }
	
	
	fHandle->curPagePos = ftell(fp); 

	
	fclose(fp);
	return RC_OK;	
}

extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	
	if (pageNum > fHandle->totalNumPages || pageNum < 0)
        	return RC_WRITE_FAILED;
	
	
	fp = fopen(fHandle->fileName, "r+");
	
	
	if(fp != NULL)
	{
		int begiPos = pageNum * PAGE_SIZE;
		if(pageNum != 0) 
		{
            fHandle->curPagePos = begiPos;
		    fclose(fp);
		    writeCurrentBlock(fHandle, memPage);
		} 
		else
		{
		    fseek(fp, begiPos, SEEK_SET);	
		    int i;
		    for(i = 0; i < PAGE_SIZE; i++) 
		    {
			   if(feof(fp)) 
			   appendEmptyBlock(fHandle);
			   fputc(memPage[i], fp);
		    }
		
		fHandle->curPagePos = ftell(fp); 
		fclose(fp);
		}
		
	}
	else
	{
		return RC_FILE_NOT_FOUND;
	}

	return RC_OK;
}

extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	
	fp = fopen(fHandle->fileName, "r+");

	
	if(fp != NULL)
	{
		appendEmptyBlock(fHandle);

	
	    fseek(fp, fHandle->curPagePos, SEEK_SET);
	
	
	    fwrite(memPage, sizeof(char), strlen(memPage), fp);
	
	
	    fHandle->curPagePos = ftell(fp);
	
	}
    else
	{
		return RC_FILE_NOT_FOUND;
	}	
	
	fclose(fp);
	return RC_OK;
}


extern RC appendEmptyBlock (SM_FileHandle *fHandle) {
	
	int seeking = fseek(fp, 0, SEEK_END);
	SM_PageHandle nullPage = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
	
	
	if( seeking != 0 )
	{
		free(nullPage);
		return RC_WRITE_FAILED;	
	} 
	else 
	{
		fwrite(nullPage, sizeof(char), PAGE_SIZE, fp);
	}
	
	free(nullPage);
	
	fHandle->totalNumPages++;
	return RC_OK;
}

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){

   //my works
  //Req 20: Check if file has less pages than numberOfPages
    if (fHandle->totalNumPages < numberOfPages)
    {
        printf("totalNumPages are less than numberofPages, hence we increase them\n" );

        //Req 21: then calculate number of pages required to increase the size to meet numberofPages
        int pagesReq = numberOfPages - fHandle->totalNumPages;
        printf("The number of pages required to increase the size = %d\n", pagesReq );
        
        //Req 22: Increases the size of the file to required size by appending required pages. 
        int i;
        for (i=0; i < pagesReq; i++)
        {
            appendEmptyBlock(fHandle);
            printf("Final total number of pages = %d\n", fHandle->totalNumPages ); 
        }
        
    }
    
    return RC_OK;
}
