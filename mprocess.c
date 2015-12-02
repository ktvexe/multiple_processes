#include <stdio.h>
#include <stdint.h>
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
	long total = 0;	
	FILE* fptr;
	long size,share_size,remain,pnum,pos;
	int *pids;
	char *readbuf,*addbuf,*readbuf_end,*newbuf;
	char* search;
	long len=strlen(argv[2]);
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
	//share file to process
	pnum = strtol(argv[3],NULL,10);	
	share_size = size / pnum;
	remain = size % pnum;
	
	//get share memory
	shmid = shmget(IPC_PRIVATE, SIZE, IPC_CREAT|0600 ) ;
    if ( shmid < 0 ){
            perror("get shm  ipc_id error") ;
            return -1 ;
    }
    //create child
	pids = (int*)malloc(pnum*sizeof(int));
	readbuf = (char*)malloc(share_size*sizeof(char));
	addbuf = (char*)malloc(len*sizeof(char));
	readbuf_end = (char*)malloc(remain*sizeof(char));
	fseek(fptr,0,SEEK_SET);
	for( i = 0;i<pnum;i++){
		//child process work
		if(!( pids[i]= vfork())){
	//		printf("child:%d\n",i);
			int count=0;
			//shared memory
			shmaddr = (char *)shmat( shmid, NULL, 0 ) ;
     		if ( (intptr_t)shmaddr == -1 ){
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
//				printf("fptr-o:%p\n",fptr);
/*				if(i!=0){
					fread(readbuf,sizeof(char),share_size,fptr-len);
				}
				else
*/				fread(readbuf,sizeof(char),share_size,fptr);
//				printf("fptr-o1:%p\n",fptr);
				pos = ftell(fptr);
				fread(addbuf,sizeof(char),len,fptr);
//				printf("position:%ld",ftell(fptr));
//				printf("readbuf:%s\n",readbuf);
//				printf("readbuf:%s\n",addbuf);
				fseek(fptr,pos,SEEK_SET);
//				printf("fptr-o2:%p\n",fptr);
				newbuf = (char*)malloc((share_size+len)*sizeof(char));
				strcpy(newbuf,readbuf);
				strcat(newbuf,addbuf);
//				printf("a\n");
				//fptr -=len;
//				printf("fptr-f:%p\n",fptr);
//				printf("b\n");
				
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
			exit(0);
		}
	}

	shmaddr = (char *)shmat( shmid, shmaddr, 0 ) ;
    if ( (intptr_t)shmaddr == -1 ){
    	perror("shmat addr error") ;
        return -1 ;
    }
	char *part = strtok(shmaddr," ");
	for(i=0;i<pnum;i++){
		printf("process%d:%s ",i+1,part);
		total += strtol(part,NULL,10);	
		part =strtok(NULL," ");
	}
	printf("total:%ld\n",total);
	shmdt(shmaddr);
	shmctl(shmid,IPC_RMID,NULL);
	fclose(fptr);
    return 0 ;

}
