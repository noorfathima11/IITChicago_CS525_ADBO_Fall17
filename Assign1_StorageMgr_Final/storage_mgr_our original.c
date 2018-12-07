#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "storage_mgr.h"
#include "dberror.h"

void initStorageManager (void){
}

//---------------------------------------------------------------------------------------------------------

RC createPageFile (char *fileName){

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
    if (initialPage == NULL)
    {
      printf("Insufficient memory\n" );
      return;
    }

    if (indexPage == NULL)
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

//--------------------------------------------------------------------------------------------------------------

RC openPageFile (char *fileName, SM_FileHandle *fHandle)
{
 //Req 4: This function has to open an existing file 
 // We repeat steps of Req 1. 
	FILE *fp;
	fp = fopen(fileName, "r+" ); //The difference is here we use "r+" mode which is opening a text file for read/write operation

    int totalSize = 0, count = 0 ;
    char ch;
 //Req 5: Should return RC_FILE_NOT_FOUND if the file does not exist.
   if(!fp)
   {
   	  printError(RC_FILE_NOT_FOUND);
      return RC_FILE_NOT_FOUND;
   }
 //Req 6: If opening is successful, then the fields of the file handle should be initialzed with the info about the opened file
   else
   {
     // We will first assign the filename and print it
   	   fHandle -> fileName = fileName;
   	   printf("The name of given file is: %s\n", fHandle -> fileName);
     
     // We will now calculate the total number of pages and print it
     //   fseek(fp, 0, SEEK_END);
     //   totalSize = ftell(fp);
     //   printf("totalSize = %d\n", totalSize ); 

       while(feof(fp)==0)
       {    
        ch = fgetc(fp);
        if(ch==EOF)
        break;
        count++;
       }

       printf("\n The total size of page is %d\n",count);
       totalSize = count;
       
       fHandle -> totalNumPages = (int) (totalSize/PAGE_SIZE);
       printf("totalNumPages = %d\n", fHandle -> totalNumPages );

     // Next we set the current position in the page to 0
       fHandle -> curPagePos = 0;
       printf("curPagePos = %d\n", fHandle -> curPagePos );

     // Now we store the book keeping info about the file our storage manager needs
       fHandle -> mgmtInfo = fp;
       printf("mgmtInfo = %p\n", fHandle -> mgmtInfo); // %p format specifier is used for void *  

       return RC_OK;
    }

}

//----------------------------------------------------------------------------------------------------------------

RC closePageFile (SM_FileHandle *fHandle)
{
  // Req 7: Close an open page file
  // close open file descriptor at fHandle->mgmtInfo
	int closeFileDescriptor = fclose(fHandle->mgmtInfo); 

  if(!closeFileDescriptor)
    return RC_OK;


  return RC_FILE_NOT_FOUND;
}

//-----------------------------------------------------------------------------------------------------------------

RC destroyPageFile (char *fileName)
{
	// Req 8: Destroy the page file
	int isDestroy = remove(fileName);
  
  if (!isDestroy)
    return RC_OK;

  return RC_FILE_NOT_FOUND;

}

//------------------------------------------------------------------------------------------------------------------

//Read Functions

RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  // Req 9: If the file has less than pageNum pages, the method should return RC_READ_NON_EXISTING_PAGE.
	if(pageNum > fHandle -> totalNumPages || pageNum < 0)
	{
		printf("File has less than %d pages, i.e pageNum pages.\n", pageNum );
		return RC_READ_NON_EXISTING_PAGE;
    }

  // Req 10: reads the pageNumth block from a file and stores its content in the memory pointed to by the memPage page handle.
  
  // Step 1: Repeat steps of Req 1 but use file descriptor for checking if file is open and pointer is available
     if (fHandle->mgmtInfo == NULL)
        return RC_FILE_NOT_FOUND;

  // Step 2: Set the file pointer at the beginning of page
     int seeking = fseek(fHandle->mgmtInfo, (pageNum+1)*PAGE_SIZE*sizeof(char), SEEK_SET);

  // Step 3 : Check if step 2 is a success
  if(seeking == 0)
    printf("File pointer successfully set at beginning of the page.\n" );
  else
  	return RC_READ_NON_EXISTING_PAGE;
 
 // Step 4; Read the file into memPage
    size_t readEntirePage = fread(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo);

 // Step 5: Update the latest pointer position
    fHandle->curPagePos = pageNum;
    printf("The updated pointer position after read = %d\n", fHandle->curPagePos );

    return RC_OK;
}

//------------------------------------------------------------------------------------------------------------------------------

int getBlockPos (SM_FileHandle *fHandle)
{
	// Req 11: Return the current page position in a file
    printf("The current page position = %d\n", fHandle->curPagePos );
    return fHandle->curPagePos;
}

//-------------------------------------------------------------------------------------------------------------------------------

RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	// Req 12: reads the first block in a file 
	  printf("Reading the first block in the file\n:" );
    return readBlock(0, fHandle, memPage);
}

//-------------------------------------------------------------------------------------------------------------------------------

RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	// Req 13: reads the last block in a file 
	  printf("Reading the last block in the file\n:" ); 
    return readBlock(fHandle->totalNumPages, fHandle, memPage);
}

//-------------------------------------------------------------------------------------------------------------------------------

RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	// Req 14: reads the previous page relative to the curPagePos in a file
	  printf("Reading the previous block in the file\n:" );
    return readBlock(fHandle->curPagePos-1, fHandle, memPage);
}

//-------------------------------------------------------------------------------------------------------------------------------

RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	// Req 15: reads the current page relative to the curPagePos in a file
	  printf("Reading the current block in the file\n:" );
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

//-------------------------------------------------------------------------------------------------------------------------------

RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	// Req 15: reads the Next page relative to the curPagePos in a file
	  printf("Reading the next block in the file\n:" );
    return readBlock(fHandle->curPagePos+1, fHandle, memPage);
}

//-------------------------------------------------------------------------------------------------------------------------------

RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  // Req 16: If the file has less than pageNum pages, the method should return RC_WRITE_FAILED.
  if(pageNum > fHandle -> totalNumPages || pageNum < 0)
  {
    printf("File has less than %d pages, i.e pageNum pages.\n", pageNum );
    return RC_WRITE_FAILED;
    }

  // Req 17: writes the pageNumth block to a file on disk 

  // Step 1: sets file write pointer to the pagenumber input by the user 
     int seeking = fseek(fHandle->mgmtInfo, (pageNum+1)*PAGE_SIZE*sizeof(char), SEEK_SET);

  // Step 2 : Check if step 2 is a success
  if(seeking == 0)
    printf("File pointer successfully set at beginning of the page.\n" );
  else
    return RC_WRITE_FAILED;
 
 // Step 3 : writing data from the memory block pointed by memPage to the file
    size_t writeEntirePage = fwrite(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo);

 // Step 4: Update the latest pointer position
    fHandle->curPagePos = pageNum;
    printf("The updated pointer position after write = %d\n", fHandle->curPagePos );


    return RC_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------

RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    // Req 18: pass current page position 
    printf("The current page position = %d\n", fHandle->curPagePos );
    return writeBlock (fHandle->curPagePos, fHandle, memPage); 
}

//-------------------------------------------------------------------------------------------------------------------------------

RC appendEmptyBlock (SM_FileHandle *fHandle)
{
  //Req 19: Increase the no of pages in the file by 1

  //Step1 : We create a last page of PAGE_SIZE bytes
  SM_PageHandle lastPage = (SM_PageHandle) calloc (PAGE_SIZE, sizeof(char));

  //Step2: We move the cursor to the last page
  int isSeeking = fseek(fHandle->mgmtInfo, (fHandle->totalNumPages + 1) * PAGE_SIZE * sizeof(char), SEEK_END);

  //Step3: Check if step 2 is a success
  if(isSeeking == 0)
    printf("Cursor successfully moved to the last page.\n" );
  else
  { 
    free(lastPage);
    return RC_WRITE_FAILED;
  }
  //Step4: writes data from the memory block pointed by lastPage to the file i.e last page is filled with zero bytes.
  size_t writeLastPage = fwrite(lastPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo); 

  //Step5: We now increment the number of pages since we added an empty page
  fHandle->totalNumPages = fHandle->totalNumPages++;
  fHandle->curPagePos = fHandle->totalNumPages;

  //Step6: We need to update the mgmtinfo
  rewind(fHandle->mgmtInfo);
  fprintf(fHandle->mgmtInfo, "%d\n" , fHandle->totalNumPages);
  
  //Step7: We update the pointer to beginning of page
  fseek(fHandle->mgmtInfo, fHandle->totalNumPages * PAGE_SIZE * sizeof(char), SEEK_SET);
  
  //Step 8: We deallocate the memory allocated for lastPage
  free(lastPage);
 
  return RC_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){

  //Req 20: Check if file has less pages than numberOfPages
    if (fHandle->totalNumPages < numberOfPages)
    {
        printf("totalNumPages are less than numberofPages, hence we increase them" );

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

//All Complete-------------------------------------------------------------------------------------------------------------------------------
