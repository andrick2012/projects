/*
@File: simHeader.h
@Author: Andrick Adhikari And Zibo Zhou

@Description: Header file for simHeader.c
 * 
 * contains methods for 
 *  - Setting up router socket
 *  - Sending packet to the specified router
 *  - Print all the router tables 
 *  -send Distance vector control packets
 *  -Update router tables according to control packets recieved

*/

#ifndef SIMHEADER_H_   /* Include guard */
#define SIMHEADER_H_

#include <sys/time.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <netdb.h> // gethostbyname - IPv4


//base port number. router number added to base-port is used to assign a port number to a router
#define BASEPORT 7000 
// maximum possible size of a table in control packet
#define MAXLINE 1024

#define DVPACKET 1// control packet
#define DATAPACKET 2//data packet

struct Packet
{
    uint8_t type; //Can be either data packets or Control packets
    uint8_t destinationRouter;// specifies the end destination of the packet
    int sourceRouter;// Stores the router where packet originated
    int distanceTable[MAXLINE];// stores tables for control packets
};

/*
 * @method: setupServerSocket: sets up  socket for the router identified by the portno
 * @arguments: portno - socket for the router will be bound to this value
 */
int setupServerSocket(int portno); 

/*
 * @method: sendPacket: sends packet to a specific router
 * @arguments
 *  @destination: specifies the destination router
 *  @pData: packet to send to the destination
 *  @sockfp: socket to used for transmitting, this socket is the source of the packet.
 */
int sendPacket(int destination, struct Packet *pData, int sockfd);

/*
 * @method : printTables : prints all the router tables in the system
 * @argmuents: 
 *  @table: pointer to the 2-D table for all the routers
 *  @numberRouter: number of all the router in the system
 */
void printTables(const int* table, int numberRouter);

/*
 * @method : printTable : prints router table of the calling router
 * @argmuents: 
 *  @table: pointer to the 2-D table for the routing
 *  @numberRouter: number of all the router in the system
 *  @router: identifier for the router
 */
void printTable(const int* table, int numberRouter, int router);

/*
 * @method: sendDVUpdate: sends router table to all the other routers
 * @arguments:
 *  @sockfd: source router socket identifier
 *  @sourceRouter: identifier for the source router of the table.
 *  @distanceMatrix: pointer to table of the router
 *  @routerNumber: total number of routers
 *  @viaMatrix: to get next hop for a particular destination if no direct route available.
 */
void sendDVUpdate (int sockfd, int sourceRouter, int *distanceMatrix, int routerNumber, int *viaMatrix);

/*
 * @method: updateDVtable : update the distance vector for a router if required.
 *  @presentTable; pointer to current DV table of a router
 *  @recvPacket: Control packet received by the router
 *  @routerNumber; total number of routers in the system
 *  @viaMatrix: to update the next hop for a particular destination.
 */
int updateDVtable(int *presentTable, struct Packet *recvPacket, int routerNumber, int *viaMatrix);



#endif // FTPHEADER_H_
