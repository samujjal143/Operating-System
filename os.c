#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "pagetable.h"

struct mainmemory
{
	int frameno;
	int status;
}frames[500];

int fifoqueue[500];
int front=-1;
int rear=-1;
int diskaccess=0;
pt pagetable;
key_t SharedKey;

void initialisept(int);
void initialiseframes(int);
void fifo(int,int);
void enqueue(int);
int dequeue(int);
void lfu(int,int);
int findmin(int);

int main(int argc,char *argv[])
{
	if(argc!=4)
	{
		printf("Incorrect number of arguments\n");
		exit(0);
	}
	else
	{
		int np=atoi(argv[1]);
		int nf=atoi(argv[2]);
		int algo=atoi(argv[3]);
		int skey=getpid();
		printf("Shared memory key is %d\n",skey);
		if(np<nf)
		{
			printf("Number of pages cannot be less than number of frames\n");
			exit(0);
		}
		else
		{
			SharedKey=skey;
			int smid=shmget(SharedKey,np*sizeof(pt),IPC_CREAT|0666);
			pagetable=(pt)shmat(smid,NULL,0);
			printf("Page Table Created\n");
			initialisept(np);
			initialiseframes(nf);
			int i,j,mark;
			while(1)
			{
				if(pagetable[0].reference==-1)
				{
					printf("Page Table Destroyed\n");
					shmctl(smid,IPC_RMID,NULL);
					printf("Disk Access: %d\n",diskaccess);
					exit(0);
				}
				else
				{
					for(i=0;i<np;i++)
					{
						int flag=0;
						if(pagetable[i].request)
						{
							mark=i;
							for(j=0;j<nf;j++)
							{
								if(!frames[j].status)
								{
									printf("Process has requested for Page=%d\n",mark);
									pagetable[mark].valid=1;
									pagetable[mark].frame=frames[j].frameno;
									pagetable[mark].dirty=0;
									pagetable[mark].reference++;
									frames[j].status=1;
									enqueue(mark);
									diskaccess++;
									printf("Allocated Page=%d to Frame=%d\n",mark,pagetable[mark].frame);
									pagetable[mark].request=0;
									break;
								}
								else
									flag++;
								if(flag==nf)
								{
									if(algo==1)
										fifo(mark,np);
									else if(algo==2)
										lfu(mark,np);
									break;
								}
							}
							printf("Resume MMU Operation\n\n");
						}
					}
				}
			}
		}
	}
	return 0;
}

void initialisept(int np)
{
	int i;
	for(i=0;i<np;i++)
	{
		pagetable[i].valid=0;
		pagetable[i].frame=-1;
		pagetable[i].dirty=0;
		pagetable[i].request=0;
		pagetable[i].reference=0;
	}
	printf("Page Table Initialised\n");
}

void initialiseframes(int nf)
{
	int i;
	for(i=0;i<nf;i++)
	{
		frames[i].frameno=i;
		frames[i].status=0;
	}
}

void fifo(int page,int np)
{
	printf("Process has requested for Page=%d\n",page);
	int victimframe=dequeue(np);
	pagetable[page].valid=1;
	pagetable[page].frame=victimframe;
	pagetable[page].dirty=0;
	enqueue(page);
	sleep(1);
	diskaccess++;
	printf("Allocated Page=%d to Frame=%d\n",page,pagetable[page].frame);
	pagetable[page].request=0;
}

void lfu(int page,int np)
{
	printf("Process has requested for Page=%d\n",page);
	int victimframe=findmin(np);
	pagetable[page].valid=1;
	pagetable[page].frame=victimframe;
	pagetable[page].dirty=0;
	sleep(1);
	diskaccess++;
	pagetable[page].reference++;
	printf("Allocated Page=%d to Frame=%d\n",page,pagetable[page].frame);
	pagetable[page].request=0;
}

void enqueue(int page)
{
    if(front==-1)
        front=0;
	rear=rear+1;
    fifoqueue[rear]=page;
}

int dequeue(int np)
{
	int victim=fifoqueue[front];
	int victimframe=pagetable[victim].frame;
	pagetable[victim].valid=0;
	pagetable[victim].frame=-1;
	if(pagetable[victim].dirty==1)
	{
		sleep(1);
		diskaccess++;
		pagetable[victim].dirty=0;
	}
	front=front+1;
	return victimframe;
}

int findmin(int np)
{
	int i,min=999,victim;
	for(i=0;i<np;i++)
	{
		if(pagetable[i].valid==1 && pagetable[i].reference<min)
		{
			min=pagetable[i].reference;
			victim=i;
		}
	}
	int victimframe=pagetable[victim].frame;
	pagetable[victim].valid=0;
	pagetable[victim].frame=-1;
	pagetable[victim].reference=0;
	if(pagetable[victim].dirty==1)
	{
		sleep(1);
		diskaccess++;
		pagetable[victim].dirty=0;
	}
	return victimframe;
}
