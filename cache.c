#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cache.h"



static cache_entry_t *cache = NULL;
static int cache_size = 0;
static int clock = 0;
static int num_queries = 0;
static int num_hits = 0;
int createcheck=0;
int fail_count1;
int fail_count2;


int cache_create(int num_entries){//creates cache
  //cache_destroy();
  //create_time=clock;
  if (createcheck==1){//checks if cache already exists
	return -1;
	}
  if (num_entries<2||num_entries>4096){//fail condition
    return -1;
	  }  
  cache=(malloc(sizeof(cache_entry_t)*num_entries));//dynamic allocation
  for (int i=0;i<cache_size;i++){
  cache[i].valid=false;
  cache[i].access_time=0;
  }
  cache_size=num_entries;
  createcheck=1;
  return 1;
      }
  
  
  


int cache_destroy(void) {//frees memory and destroys cache
  if (createcheck==0){
    return -1;
  }
  free(cache);
  cache=NULL;
  cache_size=0;
  createcheck=0;
  clock=0;
 
  return 1;
}

int cache_lookup(int disk_num, int block_num, uint8_t *buf) {//stores memory for given cache in buffer
  
  
  if(buf==NULL){
  	num_queries=num_queries+1;
      return -1;
   }
  
  int i;
  if (disk_num>16||block_num>256||disk_num<0||block_num<0||createcheck==0||cache_size<2){
    num_queries=num_queries+1;
    
  	return -1;
	
  	}
  

  for (i=0;i<=cache_size;i++){
    if (cache[i].valid==true&&cache[i].block_num==block_num&&cache[i].disk_num==disk_num){//finds cache value
      memcpy(buf,cache[i].block,256);//stores value in buffer
	num_hits=num_hits+1;
	//printf("LookUp%d %d",cache[i].disk_num,cache[i].block_num);
	clock=clock+1;//advance clock
	cache[i].access_time=clock;
	num_queries=num_queries+1;
	cache[i].valid=true;//sets valid

	return 1;
    }
    
    
    
       
  }
   
  
  
  num_queries=num_queries+1;
  
  return -1;

}
void cache_update(int disk_num, int block_num, const uint8_t *buf) {//updates the block of a certain cache value
	if (disk_num<=16&&block_num<=256){
	  int i;
	  for (i=0;i<=cache_size;i++){//sizeof wrong
	   
		if (cache[i].valid&&cache[i].block_num==block_num&&cache[i].disk_num==disk_num){//finds cache entry
		  //if (disk_num==8&&block_num==9){
		    //printf("disk: %d  %d \n",cache[i].disk_num,cache[i].block_num);
		  //}
		  memcpy((char *)cache[i].block,buf,JBOD_BLOCK_SIZE);//inserts buffer contents into cache
		  clock=clock+1;
		
		  cache[i].access_time=clock;//sets clock again
			}
	       	}
	}
}

int cache_insert(int disk_num, int block_num, const uint8_t *buf){
  //if (disk_num==8&&block_num==9){
  //printf("test1");
  //}
  if (disk_num<=16&&block_num<=256&&cache_size<=4096&&cache_size>=2&&disk_num>=0&&block_num>=0&&createcheck==1){
	//neccisary condition to proceed
    
    if (buf==NULL){	 
      return -1;
	}
    
	int t=clock;
	int i;
	for (i=0;i<=cache_size;i++){
	  if (cache[i].valid==true&&cache[i].block_num==block_num&&cache[i].disk_num==disk_num){
	   
	    return -1;
	  }
	 
	}
  
	for (i=0;i<=cache_size;i++){
	  
	  if (cache[i].access_time==0){
	    memcpy((char *)cache[i].block,buf,256);
	    clock=clock+1;
	    //printf("block\n%d\n\n\n",cache[i].block_num);
	    cache[i].block_num=block_num;
	    cache[i].disk_num=disk_num;
	    cache[i].access_time=clock;
	    cache[i].valid=true;
	    return 1;
	  }
	}
	
  
	  
	
	int j;  
	int index;
	for (j=0;j<=cache_size;j++){//find oldest value
          
	  if (cache[j].access_time<=t&&cache[j].valid==true){//if oldest value
            
	    t=cache[j].access_time;//stores oldest date
	   	
	    index=j;
	   
				}
		 
	}
	
	cache_update(cache[index].disk_num,cache[index].block_num,buf);//inserts itself into spot of oldest value
	//printf("%d----%d",cache[index].disk_num,cache[index].block_num);
	//printf("%d",cache[index].disk_num);
	//printf("%d",cache[index].block_num);
	cache[index].block_num=block_num;
	cache[index].disk_num=disk_num;
        
	
	clock=clock+1;
        //printf("%d",cache[index].valid);
        //cache[index].valid=true;
	j=clock;
	cache[index].access_time=j;

	       
	    return 1;		       		
		
  }

  
  return -1;
}

bool cache_enabled(void) {//enables cache
  if (createcheck==1){
    return true;
      }
  return false;
}

void cache_print_hit_rate(void) {
  fprintf(stderr, "Hit rate: %5.1f%%\n", 100 * (float) num_hits / num_queries);
}

