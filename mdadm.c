#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "mdadm.h"
#include "jbod.h"
#include "cache.h"
#include "net.h"




int mountcheck=-1;//check if its mounted or not
uint32_t blockid;//variable that defines the current id of the block
uint32_t diskid; //variable that defines the id of the current disk
uint32_t addy; //variable that stores the curret adress
uint8_t buf2[256];//second buffer
uint32_t addy2;//stores address for write function
uint32_t start;//stores relative position in block for read
uint32_t start2;//stores relative position in block for write
uint8_t buf3[256];//second buffe


int mdadm_mount(void) {
  //mounts disk
  if (mountcheck==-1){
  jbod_cmd_t command=JBOD_MOUNT;
  command = command <<26;//shift command by 26
  uint32_t op=command;//mount command
  printf("%d",op);
  if(jbod_client_operation(op,NULL)==0){
  //if (jbod_operation(op,NULL)==0){//conduct operation
    mountcheck=1;//stores that the disk is mounted
    return 1;//success
    
  }
  }
  
  return -1;//failure
}

int mdadm_unmount(void) {
  if (mountcheck==1){//checks if it has been mounted
    jbod_cmd_t command=JBOD_UNMOUNT;//unmount command
    command = command <<26;//move by 26
    uint32_t op=command;//specifes command as unsigned 32 int
    if (jbod_client_operation(op,NULL)==0){//unmounts
      mountcheck=-1;//sets global variable as unmounted
      return 1;//success
   

  }
  }
  
  
  return -1;//failure
  
}


int block_seek(uint32_t addr){//seeks specified block
  jbod_cmd_t command1=JBOD_SEEK_TO_BLOCK;//seeks block jbod command
  command1=command1 << 26;//shift by 26
  blockid=(addr-(diskid*65536))/256;//sets block id
  uint32_t op=(blockid|command1);//or statement to combine operations
  if (jbod_client_operation(op,NULL)==0){//seeks block
    return 1;
  }
  
  return -1;//seek failed
  

}


int disk_seek(uint32_t addr){//seeks speified disk
  jbod_cmd_t command1=JBOD_SEEK_TO_DISK;
  command1=command1 <<26;//pushes 26
  diskid=addr/65536;//sets diskid
  uint32_t opdiskid=diskid <<22;
  uint32_t op=((opdiskid)|(command1));//combines the two commands
  if (jbod_client_operation(op,NULL)==0){//seeks disk
    return 1;
  }
  
  return -1;//seek failed;
  

}

uint32_t position_relative(uint32_t address){
  return (address%256);//calculates the relative position inside of the respective block
}



int mdadm_read(uint32_t addr, uint32_t len, uint8_t *buf) {
  
  if (len>1024){//invalid input
    return -1;

   
   }

  
  if (len==0){
    return len;//special case
  }
  
  if (len!=0&&buf==NULL){
    return -1;
    //invalid input
  }
 
  uint32_t bytes_left=len;//initially sets the bytes left tothe total bytes to be read
  addy=addr;//variable to store the adresses value
  disk_seek(addy);//seeks disk
  block_seek(addy);//seeks block
 
  
 
  start=(position_relative(addr));//the initial position inside the block
  jbod_cmd_t command3=JBOD_READ_BLOCK;
  command3=command3 <<26;    //push command 26
  uint32_t op2=command3;
  jbod_operation(op2,buf2);//reads first block
  //jbod_client_operation(op2,buf2);
  uint32_t rbits=256-start;//initially sets the remaining bits in block
  
  
  //the distance traveled in the block already
      
    
  if (mountcheck==1&&(addr+len)<=1048576&&len<=1024){
    //condition to make sure parameters are good
    
    while(bytes_left!=0){//ends when all bytes have been read
      
      
    
      if (start+bytes_left<256){
	rbits=bytes_left;//condition for the last read
      }
      else{
	rbits=256-start;//remaining bits in block
      }
      
      
      memcpy(((char *)buf+(len-bytes_left)),&buf2[start],rbits);//copies memory into buffer
    
       
	addy=addy+rbits;//total adress is increased by amount of bits read in block
	bytes_left=bytes_left-rbits;


	start=0;//sets the position in block back to zero
	

	  
	
        
	
	
	if (addy%65536==0&&addy>65536){
	  int  opseekdisk=disk_seek(addy);//seeks disk
	  blockid=0;
	  if (opseekdisk==-1){//condition for if seek fails
	    return -1;
		}
	}
	 
	  

	//jbod_operation(op2,buf2);
	jbod_client_operation(op2,buf2);
	  //blockid=(addy-(diskid*65536))/256;
	  if (cache_enabled()){
	    cache_insert(diskid,blockid,buf2);
	  }
	 
	  //}
      //cache_insert(diskid,blockid,buf2);
	
	
	
	
   
      }
    
    
    return len;

   
  }	   
  
  return -1;
  }


int mdadm_write(uint32_t addr, uint32_t len, const uint8_t *buf) {//function writes the information from given buffer to the IO
  
 
  if (len==0){
  return len;//special case
  }
  
  if (len!=0&&buf==NULL){
    return -1;
    //invalid input
  }
  //printf();  
uint8_t buf3[256];
disk_seek(addr);//seeks disk
block_seek(addr);//seeks block
addy2=addr;
uint32_t bytes_left=len;
start2=(position_relative(addr));//the initial position inside the block
 
 
jbod_cmd_t command1=JBOD_WRITE_BLOCK;
command1=command1 <<26;//pushes 26
uint32_t op2=command1;
uint32_t rbits=256-start2;//initially sets the remaining bits in block
 
if (mountcheck==1&&(len<=1024)&&(addr+len<=1048576)){//condition to be met
   
   while(bytes_left!=0){//ends when all bytes have been written
    
     if (start2+bytes_left<256){
     rbits=bytes_left;//condition for the last write
	 }
   else{
     rbits=256-start2;//remaining bits in block
	}
     
     //blockid=(addy2-(diskid*65536))/256;
     if (cache_enabled()){
       if (cache_lookup(diskid,blockid,buf3)==-1){
	  
	  mdadm_read(addy2-(addy2%256),256,buf3);//reads the entire buffer from beginning to end
     }
       else{
	
	 cache_update(diskid,blockid,buf);
	 
     }
     }
       else{
	  mdadm_read(addy2-(addy2%256),256,buf3);
       }
       //else{
       //if (cache_enabled()){
	 //printf("asdfasf");
	 //cache_update(diskid,blockid,buf3);
       //}
       //}
     
     disk_seek(addy2);//seeks disk
     block_seek(addy2);//seeks block
     
     
   
     memcpy(&buf3[start2],&buf[len-bytes_left],rbits);//copies into the temorary buffer
 
     //jbod_operation(op2,buf3);
     jbod_client_operation(op2,buf3);
     //conducts the jbod operation
   
   
     addy2=addy2+rbits;//total adress is increased by amount of bits read in block
     bytes_left=bytes_left-rbits;
     start2=0;//sets the position in block back to zero
  

  	if (addy%65536==0&&addy>65536){
	  int  opseekdisk=disk_seek(addy);//seeks disk
	
	  if (opseekdisk==-1){//condition for if seek fails
	    return -1;
		}
	}
    
   }

  return len;
 }
 return -1;//fail condition
}




