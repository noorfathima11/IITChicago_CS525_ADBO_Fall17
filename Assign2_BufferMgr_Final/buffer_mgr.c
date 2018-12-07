#include<stdio.h>
#include<stdlib.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <math.h>

#define MINPAGES 0
#define NOCLIENT 0 
#define YESCLIENT 1
#define ISDIRTY 1
#define ISNOTDIRTY 0

//max frames that can reside in a buffer-pool
int sizeOfBuffer = 0;

//count of number of pages written to the disk
int pageWriteCount = 0;

//count of number of pages read from the disk
int pageReadCount = 0;

//variable the keeps track of number of frames everytime added to the bufferpool 
int hit = 0;

//a frame in buffer pool
typedef struct PageFrame
{
	PageNumber pageNumber; // position of the page in page file residing in the memory
	int clientNum;         // the number of clients that have access to the frame at the particular time
	int dirtyBit;          //  indicates if the frame data has been edited/modified by the client
	int pageHits;         //  keeps tab of least recently used page, helps LRU	
	SM_PageHandle pageData; //The data of the page in page file residing in the disc
} pf;

extern void FIFO(BM_BufferPool *const bm, pf *pg)
{
//The FIFO strategy keeps track of time each frame is created.	
	
	
	printf("FIFO Begins\n");
	int i , initialIndex, lastFrame; 
	initialIndex = pageReadCount % sizeOfBuffer;

	pf *fp = (pf *) bm->mgmtData;

	 //Going through all the frames in buffer pool
	for(i = 0; i < sizeOfBuffer; i++)
	{	//Checking if no user is using the frame to be removed
		if(fp[initialIndex].clientNum == NOCLIENT)
		{
			// We check if page is dirty
			if(fp[initialIndex].dirtyBit == ISDIRTY)
			{
				SM_FileHandle sFileHandle;
				//Opens the page file to be written
				openPageFile(bm->pageFile, &sFileHandle);

				// Writes the block to disk as  data has been modified 			
				writeBlock(fp[initialIndex].pageNumber, &sFileHandle, fp[initialIndex].pageData);
				
				// Increment write count
				pageWriteCount++;
			}
			
			// We initialize the frame's content to disk's page content
			fp[initialIndex].pageData = pg->pageData;

			fp[initialIndex].pageNumber = pg->pageNumber;

			fp[initialIndex].dirtyBit = pg->dirtyBit;

			fp[initialIndex].clientNum = pg->clientNum;

			break;
		}
		else
		{
			// If the current page frame is being used by some client, we move on to the next location
			initialIndex= initialIndex + 1;
			// Checking if initialIndex is the last frame in buffer.
			lastFrame = initialIndex % sizeOfBuffer;

			
		    if(lastFrame == 0)
		    {
		    	printf(" The Client has not used the frame and it is free to remove ");
			    initialIndex = 0;

		    }
		    else
		    {
			    printf("Frame is in use. The next frame is pointed");
			    initialIndex = lastFrame; 
		    }
		}
	}
}

extern void LRU(BM_BufferPool *const bm, pf *pg)
{	
// In LRU we keep track of number of times each frame is accessed and find whichever frame is least accessed and replace it with new frame.
	int freeFrame, eachFrame, hitNum;
	pf *fp = (pf *) bm->mgmtData;
	//Going through all the frames in buffer pool
	for(eachFrame = 0; eachFrame < sizeOfBuffer; eachFrame++)
	{
	// Checking if each page frame is used by client or not. If it is in use we cannot replace it.
		if(fp[eachFrame].clientNum !=0)
		{
			printf("Frame is in use and unable to replace it");
		}
	//If frame not in use we calculate the number of hits
		if(fp[eachFrame].clientNum == 0)
		{
			freeFrame = eachFrame;
			hitNum = fp[eachFrame].pageHits;
			break;
		}
	}	
	//Here we compare each frame with the hitNum value and if a frame is found to be having lesser than what was previously calculated then that pageFrame becomes the leasthit
	for(eachFrame = freeFrame + 1; eachFrame < sizeOfBuffer; eachFrame++)
	{
		if(fp[eachFrame].pageHits < hitNum)
		{
			freeFrame = eachFrame;
			hitNum = fp[eachFrame].pageHits;
		}
	}
	//Before replacng we fins if the data of frame is modified. 
	if(fp[freeFrame].dirtyBit == 1)
	{
		SM_FileHandle sFileHandle;
		printf("Checking if file exists or not5\n");
		if(openPageFile(bm->pageFile, &sFileHandle) ==RC_OK)
		{
 		   printf("File exists5\n");
		}      		
		else
        	printf("RC_FILE_NOT_FOUND5\n");
		// Writing if data is modified.
		if(writeBlock(fp[freeFrame].pageNumber, &sFileHandle, fp[freeFrame].pageData) == RC_OK)
		{				
		        printf("Error while writing back to the disc5\n");
				printf("RC_WRITE_FAILED5\n");
		}
		
		//Incremeting pageWriteCount
		pageWriteCount = pageWriteCount + 1;
	}
	// We initialize the frame's content to disk's page content
	fp[freeFrame].pageNumber = pg->pageNumber;
	fp[freeFrame].dirtyBit = pg->dirtyBit;
	fp[freeFrame].clientNum = pg->clientNum;
	fp[freeFrame].pageHits = pg->pageHits;
	fp[freeFrame].pageData = pg->pageData;
}

extern RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		  const int numPages, ReplacementStrategy strategy, 
		  void *stratData)
//This function is used for initializing a buffer with null values.
{
	printf("The control has entered initBufferPool\n");
	int i = 0;
	SM_FileHandle sFileHandle;
	
	//Req1: Create a buffer pool for existing page file
	  //Step1: We check if the file exists 
	  printf("Checking if file exists or not\n");
      if (openPageFile ((char *)pageFileName, &sFileHandle) == RC_OK)
        printf("File exists\n");
      else
        return RC_FILE_NOT_FOUND;

	
    
	//Step3: Create a new buffer pool with numPages page frames. We need to know how big the buffer pool needs to be
	sizeOfBuffer = numPages; 
    printf("Created new buffer pool with numPages page frames\n");

	//Step4: We allocate memory required for housing numPages of pages in the buffer pool
	pf *fp = malloc(sizeof(pf) * numPages); //size of each frame * number of frames
	printf("Memory allocated\n");	
	 
	//Req2: We allocate the page file's parameters to the bufferpool
	bm->pageFile = (char *)pageFileName;

	bm->numPages = numPages;

	

    //Req3: Req1 should happen while we are using a page replacement strategy 
	bm->strategy = strategy;
	printf("page file's parameters allocated\n");

	//Req4: The page frames should be empty
	while(i < sizeOfBuffer)
	{
		fp[i].pageData = NULL;
		fp[i].pageNumber = NO_PAGE;
		fp[i].dirtyBit = 0;
		fp[i].clientNum = 0;
		fp[i].pageHits = 0;	
		i++;
	}
    printf("Made all page frame's empty\n");
	//Since we did not write any page to disc we initilize the pageWritecount to zero;

	bm->mgmtData = fp;	
	pageWriteCount = 0;
	printf("Control going out of initBufferPool");
	
	
	return RC_OK;		
}

extern RC shutdownBufferPool(BM_BufferPool *const bm)
{
	int i = 0;
	//We first check for the validity of the buffer pool
	if (bm->numPages <= MINPAGES || !bm){
        printf("Invalid buffer pool\n" );
        return RC_ERROR;
	}
   //Req 5: We write all dirty frames (with clientNum = 0) in the buffer pool to the disc  
	forceFlushPool(bm);

	//Req 6: Check if any clients are accessing any of the page
	pf *fp = (pf *)bm->mgmtData;


    //Step1: We iterate through all the frames to check if they are pinned	
	while(i < sizeOfBuffer)
	{
		//Step2: Check if there are any clients who have requested access to the frame
		if(fp[i].clientNum == 0)
		{   
			printf("There are no pinned pages\n");
		}
		else
		{
			printf("There are pinned pages\n");
			printf("The client number %d has pinned the page\n", i);
        	return RC_ERROR;
		}
		i++;
	}

	//Req7: Free the memory allocated for page frames 
	free(fp);
	
	bm->numPages = MINPAGES;
	bm->mgmtData = NULL;

	return RC_OK;
}


extern RC forceFlushPool(BM_BufferPool *const bm)
{

	//We first check for the validity of the buffer pool
	if (bm->numPages <= MINPAGES || !bm)
	{
        printf("Invalid buffer pool\n" );
        return RC_ERROR;
	}
	//Check if any clients are accessing any of the page	
	pf *fp = (pf *)bm->mgmtData;
	
	int i =0;
		
	while( i < sizeOfBuffer) 
	{//Check if there are any clients who have requested access to the frame
		if(fp[i].clientNum == 0 && fp[i].dirtyBit == 1)
		{
			SM_FileHandle sFileHandle;
			// Opening page file available on disk
			openPageFile(bm->pageFile, &sFileHandle);
			//Step3: We write the modified page back to the page file residing in disc

			writeBlock(fp[i].pageNumber, &sFileHandle, fp[i].pageData);

			//Step4: Mark the dirty bit back to zero

			fp[i].dirtyBit = 0;
			
			//Step5: Increment the page write count to the disc
			pageWriteCount++;
		}i++;
	}	
	return RC_OK;
}

// ***** PAGE MANAGEMENT FUNCTIONS ***** //

extern RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	
	int i = 0;
	//We first check for the validity of the buffer pool
	if (bm->numPages <= MINPAGES || !bm){
		printf("check1 markdirty\n");
        printf("Invalid buffer pool\n" );
        return RC_ERROR;
	}

	//Req9: We have to mark a modified page as a dirty page
	pf *fp = (pf *)bm->mgmtData;
    
    	//Step1: We iterate over all the frames
	while(i < sizeOfBuffer)
	{   
		
		//Step2: We compare if the current page is the one to be marked as dirty.
		if(fp[i].pageNumber == page->pageNum)
		{
			
			//Step3: We mark it as dirty
			fp[i].dirtyBit = 1;
			
			return RC_OK;
		}i++;
		
	}
			
	return RC_ERROR;
}

extern RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{	
	
	int i = 0;
	//We first check for the validity of the buffer pool
	if (bm->numPages <= MINPAGES || !bm){
        printf("Invalid buffer pool\n" );
        return RC_ERROR;
	}
	//Req10: We need to inform that page is no longer needed
	pf *fp = (pf *)bm->mgmtData;

	//Step1: We iterate over all the frames
	while(i < sizeOfBuffer)
	{
		
		//Step2: We compare if the current page is the one to be unpinned
		if(fp[i].pageNumber == page->pageNum)
		{
			//Step3: We decrease the number of clients accessing the frame
			fp[i].clientNum--;
			break;		
		}i++;
				
	}
	
	return RC_OK;
}


extern RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	int i = 0;
	//We first check for the validity of the buffer pool
	if (bm->numPages <= MINPAGES || !bm)
	{
        printf("Invalid buffer pool\n" );
        return RC_ERROR;
    }

	//Req 11: We have to write the current content of the page back to the page file on disk
	pf *fp = (pf *)bm->mgmtData;

	//Step1: We iterate through all the frames
	while(i < sizeOfBuffer)
	{
		if(fp[i].pageNumber == page->pageNum)
		{
			//Step2: We check if the file exists and open it if it exists
	        SM_FileHandle sFileHandle;
	        
	       
            if (openPageFile (bm->pageFile, &sFileHandle) == RC_OK){
            	printf("File exists\n");
            }
            else
                return RC_FILE_NOT_FOUND;
			
			//Step3: We write the modified page back to the page file residing in disc
			if (writeBlock(fp[i].pageNumber, &sFileHandle, fp[i].pageData) == RC_OK)
				printf("Write Successful\n");
			else 
			{
				printf("Error while writing back to the disc\n");
				return RC_WRITE_FAILED;
			}

			//Step4: Mark the dirty bit back to zero
			fp[i].dirtyBit = ISNOTDIRTY;
			
			//Step5: Increment the page write count to the disc
			pageWriteCount = pageWriteCount + 1;
			
		}i++;
	}
	return RC_OK;	
}

// Here the function adds the page with page number pageNum to the buffer pool which is called as pinning a page.
// If the buffer pool is full, then it uses replacement strategy to replace a page in buffer with the new page being pinned. 
extern RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum)

{
	pf *fp = (pf *)bm->mgmtData;
	
	//Step1: Checking if buffer pool is empty and this is the first page to be pinned
	if(fp[0].pageNumber == -1)
	{	// Since pageNumber == -1 means the buffer is empty and this is the forst page to be pinned.
		//Step 2: We are allocating memory to this first page
		fp[0].pageData = (SM_PageHandle) malloc(PAGE_SIZE);

		// Step 3: After allocating memory we are opening the page to be read
		SM_FileHandle sFileHandle;
		openPageFile(bm->pageFile, &sFileHandle);

		//Step 4:So here we read the content of the open page in disk to the buffer pool
		ensureCapacity(pageNum,&sFileHandle);

		readBlock(pageNum, &sFileHandle, fp[0].pageData);
		//Step 5: Initializing the empty page into buffer by assigning the below values to all initbuffer function variables.
		fp[0].pageNumber = pageNum;
		//Step6: The user access count is incremented as the page is created in buffer the user will access it
		fp[0].clientNum++;
		pageReadCount = hit = 0;
		fp[0].pageHits = hit;	
		page->pageNum = pageNum;
		page->data = fp[0].pageData;
		
		return RC_OK;		
	}
	else
	{	// This executes when the buffer is neither empty nor full.
		int i;
		bool bufferComplete = true;
		// Step 1: Going through the buffer to check if buffer is not empty
		for(i = 0; i < sizeOfBuffer; i++)
		{//Step 2:Checking if buffer is not empty
			if(fp[i].pageNumber != -1)
				{
				// Step 3:checking if The page we need is already in buffer by comparing page number parameter is same.
				if(fp[i].pageNumber == pageNum)
				{
				// Step 4:Incrementing the number of client access
				fp[i].clientNum++;
				bufferComplete = false;
					
				page->pageNum = pageNum;
				page->data = fp[i].pageData;
					
				//Step 5:Incrementing the hit value as once again this page is accessed by user and so it is not a least recently userd page					
				hit++;

				if(bm->strategy == RS_LRU)
				//Step 6: Here the hit value which is incremented in the before step is updated in case of RS_LRU for further LRU ALgorithm	
				fp[i].pageHits = hit; 
					
				break;
				}	
							
			} else {
				// This exeutes when page is not found in buffer but we have to read it from disk before adding it to buffer
				// Step 1:Opening the page to be read
				SM_FileHandle sFileHandle;
				openPageFile(bm->pageFile, &sFileHandle);
				//Step 2:Allocating memory for the page's data
				fp[i].pageData = (SM_PageHandle) malloc(PAGE_SIZE);
				//Step 3: So here we read the content of the open page in disk to the buffer pool
				readBlock(pageNum, &sFileHandle, fp[i].pageData);
				//Step 4: Incrementing read count as read has happened				
				pageReadCount++;
				
				fp[i].pageNumber = pageNum;
				fp[i].clientNum = 1;
					
				
				page->pageNum = pageNum;
				page->data = fp[i].pageData;

				//Step 5:Incrementing the hit value as once again this page is accessed by user and so it is not a least recently userd page					
					hit++;
					if(bm->strategy == RS_LRU)
				//Step 6: Here the hit value which is incremented in the before step is updated in case of RS_LRU for further LRU ALgorithm	
				fp[i].pageHits = hit; 			
						
				bufferComplete = false;
				break;
			}
		}
				if(bufferComplete == true)

		{
			// A new page is created as page is not existing
			pf *nP = (pf *) malloc(sizeof(pf));		
			nP->pageData = (SM_PageHandle) malloc(PAGE_SIZE);

			//After allocating memory to data we are opening the page to be read
				SM_FileHandle sFileHandle;
				openPageFile(bm->pageFile, &sFileHandle);
				// So here we read the content of the open page in disk to the buffer pool
			
				readBlock(pageNum, &sFileHandle, nP->pageData);
				pageReadCount++;// Incrementing read count as read has happened
			// Assiging the parameters of frame for the newly created page.
				nP->pageNumber = pageNum;
				page->pageNum = pageNum;
			page->data = nP->pageData;
			// As it is newly created page it is not modified yet and as it is created for user's request clientnum is set to 1			
				nP->dirtyBit = 0;		
				nP->clientNum = 1;

			//Incrementing the hit value as once again this page is accessed by user and so it is not a least recently userd page					
					hit++;

					if(bm->strategy == RS_LRU)
						// Here the hit value which is incremented in the before step is updated in case of RS_LRU for further LRU ALgorithm	
						fp[i].pageHits = hit; 				

						

			// Call appropriate algorithm's function depending on the page replacement strategy selected (passed through parameters)
			switch(bm->strategy)
			{			
				case RS_FIFO: // Using FIFO algorithm
					FIFO(bm, nP);
					break;
				
				case RS_LRU: // Using LRU algorithm
					LRU(bm, nP);
					break;
				
				default:
					printf("\nAlgorithm Not Implemented\n");
					break;
			}
						
		}		
		return RC_OK;
	}	
}
		//This section is executed when the buffer is full.
		//As the buffer becomes full it has to replace already existing frames based on corresponding replacement strategies.

		
// ***** STATISTICS FUNCTIONS ***** //

// This function returns an array of frames page number in the buffer pool.
extern PageNumber *getFrameContents (BM_BufferPool *const bm)
{
	PageNumber *contentFrame = malloc(sizeof(PageNumber) * sizeOfBuffer);
	pf *fp = (pf *) bm->mgmtData;
	
	int i = 0;
	// Going through all the pages in the buffer pool and assigning contentframe value to page's page number value.
	while(i < sizeOfBuffer) {
		if(fp[i].pageNumber != -1)
		{
			contentFrame[i] = fp[i].pageNumber;
			
		}
		else
		{
		    contentFrame[i] = NO_PAGE;
		}i++;	
	}
	return contentFrame;
}


extern bool *getDirtyFlags (BM_BufferPool *const bm)
// Function returns an array of bools, each element represents the dirtyBit of the respective page.
{
	bool *dirtyFlags = malloc(sizeof(bool) * sizeOfBuffer);
	pf *fp = (pf *)bm->mgmtData;
	
	int i;
	// Going throu all frame in buffer and setting dirtyFlags' value to TRUE if page is dirty else FALSE, this is a boolean array when dirtybit = 1 page is dirty
	for(i = 0; i < sizeOfBuffer; i++)
	{
		dirtyFlags[i] = (fp[i].dirtyBit == 1) ? true : false ;
	}	
	return dirtyFlags;
}


extern int *getFixCounts (BM_BufferPool *const bm)
// Function returns an array of ints (of size numPages) where the ith element is the fix count of the page stored in the ith page frame.
{
	int *fix_count = malloc(sizeof(int) * sizeOfBuffer);
	pf *fp= (pf *)bm->mgmtData;
	
	int i = 0;
	// Iterating through all the pages in the buffer pool and setting fixCounts' value to page's fixCount
	while(i < sizeOfBuffer)
	{
		if(fp[i].clientNum != -1)
		{
		fix_count[i] =fp[i].clientNum;
		}			
		else
		{
		fix_count[i] = 0;
		}		
		i++;
	}	
	return fix_count;
}


extern int getNumReadIO (BM_BufferPool *const bm)
// Function returns the total number of pages that have been read from disk from initbuffer function.
{
	
	printf("The total number of pages that have been read from disk %d",pageReadCount++);
	return pageReadCount++;
}


extern int getNumWriteIO (BM_BufferPool *const bm)
// Function returns the number of pages written to the page file beginning from initbuffer function
{	
	printf("The total number of pages written to the page file %d ", pageWriteCount);
	return pageWriteCount;
}
