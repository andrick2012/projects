/*
@File: simHeader.c
@Author: Andrick Adhikari And Zibo Zhou

@Description: Header file for simHeader.c
 * 
 * contains implementations for methods defined in simHeader.h 
 *  - Setting up router socket
 *  - Sending packet to the specified router
 *  - Print all the router tables 
 *  -send Distance vector control packets
 *  -Update router tables according to control packets recieved

*/

#include "simHeader.h"


int setupServerSocket(int portno){
    
  int sockfd; 
  struct sockaddr_in servaddr; 
  
  // Creating socket file descriptor 
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
    perror("socket creation failed"); 
    exit(EXIT_FAILURE); 
  } 
  
  memset(&servaddr, 0, sizeof(servaddr)); 
   
  
  // Filling server information
  // The given port on this computer
  servaddr.sin_family = AF_INET; // IPv4 
  servaddr.sin_addr.s_addr = INADDR_ANY; 
  servaddr.sin_port = htons(portno); 
  
  // Bind the socket with the server address 
  if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) { 
      perror("bind failed"); 
      exit(EXIT_FAILURE); 
    } 
  
  return sockfd; // for each of the router and tester
} // Like new ServerSocket in Java

int sendPacket(int destination, struct Packet *pData, int sockfd)
{
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(BASEPORT + destination); // sum gives the destination port no.

    // Setup the server host address
    struct hostent *server;
    server = gethostbyname("localhost");
    if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
    }
    memcpy(&servaddr.sin_addr.s_addr, server->h_addr, server->h_length);
  
    (pData->type == DVPACKET)?printf("DV update packet"):printf("\nData packet");
    printf("-> sent to %d Router\n",destination);    
    
    // send the packet to destination router
    return sendto(sockfd, pData, sizeof(*pData), 
	 0, (struct sockaddr *) &servaddr, sizeof(servaddr)); 
}

void printTables(const int* table, int numberRouter){
    // prints all the distance vectors for all the routers
    printf("\n");
    for (int i = 0 ; i < numberRouter; i++)
    {
        printf("Table for Router : %d\n", i);
        for(int j= 0; j < numberRouter; j++)
        {
            printf("| %d ", table[i* numberRouter + j]);
        }
        printf("|\n");
    }
}
void printTable(const int* table, int numberRouter, int router)
{
    printf("Table for Router: %d updated ->", router);
    for(int j= 0; j < numberRouter; j++)
    {
        printf("| %d ", table[j]);
    }
    printf("|\n");
}
void sendDVUpdate (int sockfd, int sourceRouter,int *distanceMatrix, int routerNumber, int *viaMatrix)
{
    // control packet informations
    struct Packet dvUpdatePacket;
    dvUpdatePacket.type = DVPACKET;
    dvUpdatePacket.sourceRouter = sourceRouter; // specifies the origin of distance vector
    
    for(int j = 0; j < routerNumber; j++)
    {   
        //fill up the distance vector in the packet
        dvUpdatePacket.distanceTable[j] = *(distanceMatrix +j);
        
    }
   
    for(int i = 0; i < routerNumber; i++)
    {
        // send to all the routers in the system except the origin router
        if(i != sourceRouter)
        {
            if(dvUpdatePacket.distanceTable[i] == -1)
            {
                //if no possible routes exist.
                printf("\nDV Update from Router:%d cannot be sent to Router: %d -> No link\n",sourceRouter,i);
            }
            else
            {
                // destination router is changed for each of the router
                dvUpdatePacket.destinationRouter = i;
                if(*(viaMatrix + i) == -1) // send directly if viaMatrix has -1 (direct hop to destination exists)
                sendPacket(i, &dvUpdatePacket, sockfd);
                else// send to next hop if direct route doesnt exist.
                sendPacket(*(viaMatrix + i), &dvUpdatePacket, sockfd);    
            }            
        }        
    }
    
}

int updateDVtable(int *presentTable, struct Packet *recvPacket, int routerNumber, int *viaMatrix)
{
    // base distance to the the router to which DV table belongs
    int viaBaseDistance = *(presentTable + recvPacket->sourceRouter);
    int changed = -1;
    
    for(int i = 0; i < routerNumber; i++)
    {
        // if the received DV table doesn't has a route to a router, then skip. 
        if(recvPacket->distanceTable[i] == -1)
        {  
            continue;
        }
        
        // calculate cost of new route via DV sender router
        int fromNewRouterDistance = recvPacket->distanceTable[i] + viaBaseDistance;
        int currentDistance = *(presentTable + i);
        //update distance vector and viaMatrix(for next hop) if new route is less costly
        if(fromNewRouterDistance < currentDistance || currentDistance == -1)
        {
            *(presentTable + i) = fromNewRouterDistance;
            *(viaMatrix + i) = recvPacket->sourceRouter;
            int nextHop = *(viaMatrix + i);
            //reduce the via Matrix to next possible HOP
            while(*(viaMatrix + nextHop) != -1)
            {
                nextHop = *(viaMatrix + nextHop);
            }
            *(viaMatrix + i) = nextHop;
            changed = 1;
        }
        
    }
    // if changed then print the updated table
    //returns 1 if table changed , -1 if no changes
    return changed;
}