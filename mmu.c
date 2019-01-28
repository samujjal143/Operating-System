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

struct memtrace
{
	char address[10];
	char mode[1];
}entry[500];

int tracecount=0;
int pagehit=0;
int pagefault=0;
key_t Sharedkey;
pt pagetable;

void readfile();
void servicerequest(int,int,int);
int calculatepageno(char*,int);
void printpt(int);

int main(int argc,char *argv[])
{
	if(argc!=6)
	{
		printf("Incorrect number of arguments\n");
		exit(0);
	}
	else
	{
		int np=atoi(argv[1]);
		int page=atoi(argv[2]);
		int offset=atoi(argv[3]);
		int skey=atoi(argv[4]);
		int num=atoi(argv[5]);
		Sharedkey=skey;
		int smid=shmget(Sharedkey,np*sizeof(pt),IPC_CREAT|0666);
		pagetable=(pt)shmat(smid,NULL,0);
		printf("MMU is attached with Page Table\n");
		printpt(num);
		printf("\n");
		readfile();
		servicerequest(page,np,num);
		pagetable[0].reference=-1;
		shmdt(pagetable);
		printf("MMU is detached from Page Table\n");
		printf("Page Hits: %d\nPage Faults: %d\n",pagehit, pagefault);
	}
	return(0);
}

void printpt(int num)
{
	int i;
	printf("Page Table\n");
	for(i=0;i<num;i++)
		printf("%d: Valid=%d Frame=%d Dirty=%d Requested=%d\n",i,pagetable[i].valid,pagetable[i].frame,pagetable[i].dirty,pagetable[i].request);
}

void readfile()
{
	FILE *fp;
	char line[20];
	char *item;
	fp=fopen("memtrace.txt","r");
	while(fgets(line,20,fp))
	{
		item=strtok(line," ");
		strcpy(entry[tracecount].address,item);
		item=strtok(NULL,"\n");
		strcpy(entry[tracecount].mode,item);
		tracecount++;
	}
}

void servicerequest(int page,int np,int num)
{
	int i,pageno[tracecount];
	for(i=0;i<tracecount;i++)
	{
		pageno[i]=calculatepageno(entry[i].address,page);
		if(pageno[i]>=np)
		{
			printf("Page is outside memory bound\n");
			exit(0);
		}
		else
		{
			printf("Request for Page=%d in Mode=%c\n",pageno[i],entry[i].mode[0]);
			if(!pagetable[pageno[i]].valid)
			{
				printf("Not present in Page Table:Page Fault\n");
				pagefault++;
				pagetable[pageno[i]].request=getpid();
				printf("MMU Paused:Control given to OS\n");
				while(pagetable[pageno[i]].request!=0)
				{
				}
			}
			else
			{
				pagehit++;
				pagetable[pageno[i]].reference++;
				printf("Present in Page Table:Page Hit\n");
			}
			if(entry[i].mode[0]=='W')
				pagetable[pageno[i]].dirty=1;
			printpt(num);
			printf("\n");
		}
	}
}

int calculatepageno(char *addr,int page)
{
	int i,j=0,pno=0;
	int len=strlen(addr);
	int addressinbin[4*(len-2)];
	for(i=2;i<len;i++)
	{
       	if(addr[i]=='0')
           	addressinbin[j]=addressinbin[j+1]=addressinbin[j+2]=addressinbin[j+3]=0;
		else if(addr[i]=='1')
		{
           	addressinbin[j]=addressinbin[j+1]=addressinbin[j+2]=0;
			addressinbin[j+3]=1;
		}
       	else if(addr[i]=='2')
		{
           	addressinbin[j]=addressinbin[j+1]=addressinbin[j+3]=0;
			addressinbin[j+2]=1;
		}
		else if(addr[i]=='3')
		{
			addressinbin[j]=addressinbin[j+1]=0;
			addressinbin[j+2]=addressinbin[j+3]=1;
        }
       	else if(addr[i]=='4')
		{
			addressinbin[j]=addressinbin[j+2]=addressinbin[j+3]=0;
			addressinbin[j+1]=1;
		}
       	else if(addr[i]=='5')
		{
			addressinbin[j]=addressinbin[j+2]=0;
			addressinbin[j+1]=addressinbin[j+3]=1;
		}
		else if(addr[i]=='6')
		{
			addressinbin[j]=addressinbin[j+3]=0;
			addressinbin[j+1]=addressinbin[j+2]=1;
		}
		else if(addr[i]=='7')
		{
           	addressinbin[j]=0;
			addressinbin[j+1]=addressinbin[j+2]=addressinbin[j+3]=1;
		}
       	else if(addr[i]=='8')
		{
			addressinbin[j+1]=addressinbin[j+2]=addressinbin[j+3]=0;
			addressinbin[j]=1;
		}
       	else if(addr[i]=='9')
		{
			addressinbin[j+1]=addressinbin[j+2]=0;
			addressinbin[j]=addressinbin[j+3]=1;
		}
		else if(addr[i]=='A'||addr[i]=='a')
       	{
			addressinbin[j+1]=addressinbin[j+3]=0;
			addressinbin[j]=addressinbin[j+2]=1;
		}
        else if(addr[i]=='B'||addr[i]=='b')
		{
			addressinbin[j+1]=0;
			addressinbin[j]=addressinbin[j+2]=addressinbin[j+3]=1;
        }
       	else if(addr[i]=='C'||addr[i]=='c')
		{
			addressinbin[j+2]=addressinbin[j+3]=0;
			addressinbin[j]=addressinbin[j+1]=1;
		}
		else if(addr[i]=='D'||addr[i]=='d')
       	{
			addressinbin[j+2]=0;
			addressinbin[j]=addressinbin[j+1]=addressinbin[j+3]=1;
		}
		else if(addr[i]=='E'||addr[i]=='e')
		{
			addressinbin[j+3]=0;
			addressinbin[j]=addressinbin[j+1]=addressinbin[j+2]=1;
       	}
      	else if(addr[i]=='F'||addr[i]=='f')
			addressinbin[j]=addressinbin[j+1]=addressinbin[j+2]=addressinbin[j+3]=1;
		j+=4;
	}
	j=0;
	for(i=page-1;i>=0;i--)
	{
		pno+=addressinbin[i]*pow(2,j);
		j++;
	}
	return pno;
}
