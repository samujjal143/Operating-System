# Operating-System

****PAGE TABLE SIMULATION****

{Non-Conventional functions/system calls used}
-> shmget(key,shm_size,flag) - Create shared memory segment of SIZE=shm_size.
-> shmat(shm_id,NULL,0) - Attach with shared memory segment with ID=shm_id.
-> shmdt(shm_ptr) - Detach from shared memory segment pointed by shm_ptr.
-> shmctl(shm_id,IPC_RMID,NULL) - Remove shared memory segment with ID=shm_id.

[MMU Program Flow]
1. Take "Number of Pages", "Number of Page bits", "Number of Offset bits", "Shared Memory Key" and "Number of Page Table Entries to print" from command line argument.
2. Attach the MMU process with the shared memory.
3. Read the Trace File and store it in a structure.
4. Read the stored entries from the structure one by one, and calculate the page number using the page number bits.
5. 		Check if that page in in main memory using the "Valid" field of page table.
6. 		If "Valid" field is non zero, Increment "Page Fault Counter" by 1.
7. 			Store pid of MMU in the "Requested" field of page table, to let OS know that the page is requesting to be serviced.
8.			Wait in an infinite loop unless OS sets the "Requested" field to 0.
9. 		Else Increment the "Page Hit Counter" by 1.
10.		If "Mode" field of the storage structure is "W", set "Dirty" field of the page table entry as 1.
11.		Print Page Table.
12. After servicing all the pages, detach the MMU from the shared memory.

[OS Program Flow]
1. Take "Number of Pages", "Number of Frames", "Page Replacement Mode (FIFO/LFU)" from command line argument.
2. Use OS pid as the shared memory key.
3. If (Number of Pages < Number of Frames), terminate.
4. Create shared memory (page table) using shared memory key.
5. Attach OS process with shared memory.
6. Initialise Page Table entries and Frames.
7. Run an infinite loop.
8.		Check if "Requested" field of any page table entry is non zero or not.
9.		If it is non zero, then that particular page is not in the main memory. So it has to be allocated a frame.
10.			Search for an empty frame.
11.			If such a frame is found, set "Frame No." field of page table entry equal to that frame number.
12.				Set "Valid" field as 1, "Dirty" field as 0, "Requested" field as 0.
13.				Enqueue that page in a queue (for FIFO).
14.				Increment the "Reference" field by 1 (for LFU) and increment the "Disc Access counter" by 1.
15.   		If an empty frame is not found, call a "Page Replacement Algorithm" function, according to the mode entered earlier.

--FIFO Flow--
1. Dequeue the queue, and store the frame number of the deleted page.
2. Set "Valid" field of victim page as 0, "Frame No." field as -1, "Dirty" field as 0.
3. Set the "Frame No." field of the requested page with the frame number of the deleted page.
4. Set the "Valid" field of the new page as 1, "Dirty" field as 0, "Requested" field as 0.
5. Enqueue the new page into the queue.
6. Increment the "Disc Access Counter" by 1.

--LFU Flow--
1. Find a page whose "Valid" field is set to 1, such that the "Reference" field is minimum among all such pages.
2. Store the frame number of the page.
3. Set "Valid" field of victim page as 0, "Frame No." field as -1, "Dirty" field as 0, "Reference" field as 0.
4. Set the "Frame No." field of the requested page with the frame number of the deleted page.
5. Set the "Valid" field of the new page as 1, "Dirty" field as 0, "Requested" field as 0.
6. Increment the "Reference" frame by 1 and the "Disc Access Counter" by 1.
