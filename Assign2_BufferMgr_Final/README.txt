
=========================
#   Problem Statement   #
=========================

The purpose of the assignment is the implementation of the buffer manager whose function is to maintain 
a fixed number of  Pages in the memory that are present in the storage manager.
The pages that are in the memory of the Buffer manager are called page frames.
The Buffer pool comprises of page file and page frames.
Each file is assigned to a single Buffer Pool. And various page replacement strategies such as FIFO and LRU are being implemented.


STEPS TO RUN THE SCRIPT 
=========================

1) Go to the required directory path i.e to assignment_2 on the terminal.

2) To check see the listed files type ‘ls’ command and see the correct files are present in the directory.

3) Use the command ‘make clean’ so as to clear the old compiled .o type files.

To run the given test cases:
————————————————————————————
Compile : make
Run : make run_test1

1) The above command ‘make’ is used compile all the files and make object files.
2) The ‘Run’ Command is used to execute the Test File  



=========================
#         Logic         #
=========================

Attributes Definition :
————————————————————————————
The following attributes and variable are defined and used in the program

1. pageNum      - It defines the number of pages
2. frameNum     - It defines the number of page frames present in page file.
3. dirty        - It sets the dirty bit of the frame .If the bit is set to 1 then it is dirty.
4. fixCount     - Keeps the record of the pinned/unpinned request.
5. pageReadCount - Keeps the record of pages that has been read.
6. pageWriteCount - It keeps the total number of pages that has been written.
7. contentFrame -   array in which the nth page was stored in the nth page frame.
8. getDirtyFlags -  array where the ith element set to true(1) if that page frame is found to be dirty 



1. BUFFER POOL FUNCTIONS
===========================
These functions are implemented to create buffer pool for the given page file on disk.
When the page is present in the disk the buffer pool is created and this assignment is including functions from the Storage Manager to perform operations on page File on Disk.

1.initBufferPool()
* This function is used to create a new buffer in the memory.
* The number of pages that a buffer pool can store is defined by parameter numPages.
* pageFileName defines the name of page file that are being cached in memory.
* Page replacement Strategies such as LRU,FIFO,LFU have been used by the buffer pool

2.shutdownBufferPool()
* It closes/destroys the buffer pool from the memory.
* It clears up all the memory taken by the buffer pool 
* In addition to this a forceFlushPool() function is being called which is used to write 

3. forceFlushPool()
* This function is used to write all the pages that are modified(dirty pages) I.e whose dirtyBit=1 to the disk.
* If the file is not existing then RC_ERROR is Returned.
* It checks the through the frame list and on encountering the frame whose dirty bit is flagged as 1 then:
  a. It writes the data  to the disk.
  b. It again marks the dirty bit of the Frame as 0.
  c. It then increases the counter value of pageWriteCount by 1
  d. On iteration of all the frames if no error is detected, the code RC_Ok is returned




Page Management Functions :
———————————————————————————

pinPage:
1. This function initially Reads the page from disk and initializes page frame's content in the buffer pool
2. It then calls the different strategies functions like pinpage_FIFO, pinpage_LRU as defined in the code structure.
3. It Pins The page through the given Parameter   
3. These functions pins the page with the given PageNumber pageNum.
4. And If the Page is found in the memory then It increases the Hit Count which can be used in case of LRU algorithm to determine the least recently used page.
5. The different strategies used by the Buffer Manager keeps in account the page that is being requested and providing the details of that page to the client


unpinPage:
1. This function unpins a page from the memory i.e. removes a page from the memory.
2. It first iterates through all the page frames  and compares that if the current page is to be unpinned or not.
3. If the current page is to be unpinned then we decrease the number of clients accessing the frame.
4. If the page is Unpinned then RC_OK is returned 



markDirty:
1. This function first checks the validity of the buffer pool, if it is not valid then RC_ERROR is returned.
2. It then Iterated through all page frames.
3 Then It checks if the current page is the one to be marked as dirty.
4.If the current page is marked dirty then the dirty bit is set to 1 and RC_OK is returned 


forcePage:
1. It iterates through all the page frames and then it checks We check if the file exists and open it if it exists.In case of non existent of file RC_ERROR is returned.
2. We write the modified page back to the page file residing in disc. And the write operation is perfomed.
3. Then the dirty bit is again marked back to zero and page write count to disc is incremented.
4. Finally on these successful operations RC_OK is returned.



Statistics Functions:
————————————————————————
The statistics functions gives the description of the buffer pool and its contents.

getFrameContents:
1. contentFrame contains the frame contents and is present in BM_BufferPool->mgmtData which includes the array in which the nth page was stored in the nth page frame.
2. Whenever a new page frame is encountered then it is updated.
3. It returns the contentFrame array.


getDirtyFlags:
1. This function is used to return an array where the ith element set to true(1) if that page frame is found to be dirty else it set to false(0)
2. Thus it returns the array of Booleans Indicating whether the page frame is dirty or not.
3. The array is stored in the BM_bufferPool->mgmtData->dirtyFlags and it iterates over this array to find out whether the value is set to true(to find if page is dirty or not)



getFixCounts:
1. This function is used to return the array of ints
2. The array is stored in the BM_bufferPool->mgmtData->fixedCounts.The array has a structure where the nth element is the fixed count of the ith page frame .
3. The count is populated by iteration over the frames and then accounting the fix_count value of each frame.


getNumReadIO:
1. The getNumReadIO Is used to give the information regarding the number of pages that are being read from the disk after the Buffer pool has been initialized.
2. It increments the pageReadCount value after encountering the pages that has been read. And returns the value of pageReadCount.


getNumWriteIO:
1. The getNumWriteIO Is used to give the information regarding the number of pages that are being written to the page file after the Buffer pool has been initialized.
2. It returns the pageWriteCount that is total number of pages that has been written.


The Page Replacement Strategies:
————————————————————————————————

FIFO():
FIFO page replacement strategy is implemented through this function.

1. It uses the concept of queue where the page that comes in first in the buffer and that will be replaced first once the capacity of the buffer is reached out to be full.
2. It iterates through all the page frames and once the page is located, then the content of the page frame are written to the page file on disk.
3. The new page is added to that location.
4. And on on writing the page contents successfully RC_OK is returned.

LRU() Least Recently Used:
1. It checks for the existent of the page in the memory and if the page is not found Then it gives The error RC_ERROR.
2. It is based on the concept that the page frame that has not been used for the longest time will be removed first.
3. The object hitNum is used to track the count of the page frames that are being pinned and accessed by the client.
4. The hitNum with the minimum value is selected and that page is ready to be removed. 
5. After finding the page then the content of the page frame are then written on the page file on the disk. And then RC_OK is returned.








