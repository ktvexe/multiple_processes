#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <error.h>
#include <string.h>
#define SIZE 1024

int main(int argc,char *argv[]){

    static int shmid ;
    static char *shmaddr ;
    struct shmid_ds buf ;
    int flag = 0 ;
    int pid ,i;
	int total;	
	FILE* fptr;
	long size,share_size,remain,pnum;
	int *pids;
	char *readbuf,*readbuf_end,*newbuf;
	char* search;
	//check input
	if(argc !=4){
		perror("Input error!!");
		return -1;
	}
	//open file
	fptr = fopen(argv[1],"r");
	if(!fptr){
		perror("open file failed");
		return -1;
	}
	//check size of file
	fseek(fptr,0,SEEK_END);
	size =ftell(fptr);
	printf( "size: %ld\n" , size );
	//share file to process
	pnum = strtol(argv[3],NULL,10);	
	share_size = size / pnum;
	remain = size % pnum;
	printf("share_size :%ld\n",share_size);
	printf("remain :%ld\n",remain);
	
	//get share memory
	shmid = shmget(IPC_PRIVATE, SIZE, IPC_CREAT|0600 ) ;
    if ( shmid < 0 ){
            perror("get shm  ipc_id error") ;
            return -1 ;
    }
    //create child
	pids = (int*)malloc(pnum*sizeof(int));
	readbuf = (char*)malloc(share_size*sizeof(char));
	readbuf_end = (char*)malloc(remain*sizeof(char));
	fseek(fptr,0,SEEK_SET);
	for( i = 0;i<pnum;i++){
		//child process work
		if(!( pids[i]= vfork())){
			printf("child:%d\n",i);
			int count=0;
			//shared memory
			shmaddr = (char *)shmat( shmid, NULL, 0 ) ;
     		if ( (int)shmaddr == -1 ){
        	    perror("shmat addr error") ;
           		return -1 ;
        	}
			
			//last process
			if(i==(pnum-1)){
				fread(readbuf,sizeof(char),share_size,fptr);
				fread(readbuf_end,sizeof(char),remain,fptr);
//				printf("readbuf:%s",readbuf);
//				printf("%s\n",readbuf_end);
				newbuf = (char*)malloc((share_size+remain)*sizeof(char));
				strcpy(newbuf,readbuf);
				strcat(newbuf,readbuf_end);
				while(1){
					search=strstr(newbuf,argv[2]);
					if(strstr(newbuf,argv[2])){
						newbuf=search+strlen(argv[2]);
						count++;
					}
					else
						break;
				}
//				printf("count:%d\n",count);
        		char countc[10];
				sprintf(countc,"%d ",count);
				strcat( shmaddr,countc ) ;
        		shmdt( shmaddr ) ;
			}
			//normal process
			else{
				fread(readbuf,sizeof(char),share_size,fptr);
//				printf("readbuf:%s\n",readbuf);
				while(1){
					search=strstr(readbuf,argv[2]);
					if(strstr(readbuf,argv[2])){
						readbuf=search+strlen(argv[2]);
						count++;
					}
					else
						break;
				}
//				printf("count:%d\n",count);
        		char countc[10];
				sprintf(countc,"%d ",count);
				strcat( shmaddr,countc ) ;
        		shmdt( shmaddr ) ;
			}
			exit(0);
		}
	}

	shmaddr = (char *)shmat( shmid, shmaddr, 0 ) ;
    if ( (int)shmaddr == -1 ){
    	perror("shmat addr error") ;
        return -1 ;
    }
	char *part = strtok(shmaddr," ");
	for(i=0;i<pnum;i++){
		printf("process%d:%s ",i+1,part);
		total += strtol(part,NULL,10);	
		part =strtok(NULL," ");
	}
	printf("total:%d",total);
	shmdt(shmaddr);
	shmctl(shmid,IPC_RMID,NULL);

    return 0 ;

}
