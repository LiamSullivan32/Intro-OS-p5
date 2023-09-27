#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "net.h"
#include "jbod.h"

/* the client socket descriptor for the connection to the server */
int cli_sd = -1;
//uint16_t rett=0;//stores return value to be used in the operation function
//uint16_t total_read;
//int total_written;
//int total=HEADER_LEN;


/* attempts to read n (len) bytes from fd; returns true on success and false on failure. 
It may need to call the system call "read" multiple times to reach the given size len. 
*/
static bool nread(int fd, int len, uint8_t *buf) {
int temp=0;
int amount_read=0;


while (amount_read<len){

temp = read(fd,(char *)buf+amount_read,len-amount_read);//stores -1 or amount read

if (temp==-1){//fail condition

  return false;
}
amount_read = amount_read + temp;//increment to end loop
if (temp==8){//header

  return true;
}
  }
  
return true;  
}
 

/* attempts to write n bytes to fd; returns true on success and false on failure 
It may need to call the system call "write" multiple times to reach the size len.
*/
static bool nwrite(int fd, int len, uint8_t *buf) {
int temp=0;
int amount_written=0;
 //r = write(fd,buf,len); //size
 
 while (amount_written<len){
   
   temp=write(fd,(char *)buf+amount_written,len-amount_written);//sets output of write
   if (temp==-1){//write failed
   
   	return false;
   	
   }
   amount_written=amount_written+temp;//total written to end loop
   
   if (amount_written==8){//Header
   	return true;
 }
 }
 return true;  
		
  }

/* Through this function call the client attempts to receive a packet from sd 
(i.e., receiving a response from the server.). It happens after the client previously 
forwarded a jbod operation call via a request message to the server.  
It returns true on success and false on failure. 
The values of the parameters (including op, ret, block) will be returned to the caller of this function: 

op - the address to store the jbod "opcode"  
ret - the address to store the return value of the server side calling the corresponding jbod_operation function.
block - holds the received block content if existing (e.g., when the op command is JBOD_READ_BLOCK)

In your implementation, you can read the packet header first (i.e., read HEADER_LEN bytes first), 
and then use the length field in the header to determine whether it is needed to read 
a block of data from the server. You may use the above nread function here.  
*/
static bool recv_packet(int sd, uint32_t *op, uint16_t *rett, uint8_t *block) {
  
  int16_t packsize=HEADER_LEN;
  uint8_t packet[264];//packet
  bool n;
  
  n=nread(sd,264,packet);
  
  
  
  //check if nread failed
  if (!n){//read failed
    return false;
  }
  
  //packsize from packet
  memcpy(&packsize, packet, 2);//store packetsize
  packsize=ntohs(packsize);//convert order

  //op
  uint32_t net_op;//temp operation
  memcpy(&net_op,(char *)packet+2,4);//store op
  *op=ntohl(net_op);//reorder

  //return
  uint16_t rett2;//return temp
  memcpy(&rett2,(char *)packet+6,2);//store return
  *rett=ntohs(rett2);//reorder

  //if read convert block   
  if (packsize > HEADER_LEN){//if it is write
    memcpy(block,(char *)packet+HEADER_LEN,256);//fill block
  }
	     
  return true;
}



/* The client attempts to send a jbod request packet to sd (i.e., the server socket here); 
returns true on success and false on failure. 

op - the opcode. 
block- when the command is JBOD_WRITE_BLOCK, the block will contain data to write to the server jbod system;
otherwise it is NULL.

The above information (when applicable) has to be wrapped into a jbod request packet (format specified in readme).
You may call the above nwrite function to do the actual sending.  
*/
static bool send_packet(int sd, uint32_t op, uint8_t *block) {
  uint16_t packsize=HEADER_LEN;
  uint8_t packet[264];//packet
  uint32_t tempop;//temp operation
  
  //op
  tempop=htonl(op);
  memcpy((char *)packet+2, &tempop, 4);//set operation
  
  //block  
  if (block!=NULL){
    packsize=packsize+256;
    memcpy((char *)packet+HEADER_LEN,block,256);//set block
  }
  
  //packsize
  uint16_t size=htons(packsize);          
  memcpy(packet,&size,2);//set size
  
  bool n;
  n=nwrite(sd,packsize,packet);//write
  if (!n){//fail condition
    return false;
  }
    
  return true;
}

/* attempts to connect to server and set the global cli_sd variable to the
 * socket; returns true if successful and false if not. 
 * this function will be invoked by tester to connect to the server at given ip and port.
 * you will not call it in mdadm.c
*/
bool jbod_connect(const char *ip, uint16_t port) {
 
  struct sockaddr_in add;//address structure
  add.sin_family = AF_INET;
  add.sin_port=htons(port);//PROLLY RIGHT
  
 
  if (inet_aton(ip,&add.sin_addr)==0){
   
   return false;
   }
  //create socket
  cli_sd=socket(AF_INET, SOCK_STREAM,0);
  if (cli_sd==-1){
    
    return false;
  }
 //connect
  int a=connect(cli_sd, (const struct sockaddr *)&add,sizeof(add));
  
  //connection failed
  if (a==-1){
 
    return false;
  }
  
  return true;
}




/* disconnects from the server and resets cli_sd */
void jbod_disconnect(void) {
  close(cli_sd);
  cli_sd=-1;
}
//disconect



  
int jbod_client_operation(uint32_t op, uint8_t *block) {
  uint16_t rett=0;//return value
  bool r;
  bool s;
  
  //uint32_t op_copy = op;
  s=send_packet(cli_sd, op, block);//send packet
  r=recv_packet(cli_sd, &op, &rett, block);//recieve packet

  if (rett==-1){//fail condition
  	return -1;
  	}
  //op=ntohs(op);
  if (s == true  && r == true){
    return 0;
  }
 
  return -1;

}
