#include "simHeader.h"

int main(int argc, char ** argv)
{
    // Arguments require source and destination routers
    if(argc != 3)
    {
        fprintf(stderr,"Usage: ./tester <source router> <destination router>\n");
        exit(0);
    }
    int source = atoi(argv[1]);
    int destination = atoi(argv[2]);
    printf("Packet pushed into Source router %d for destination router %d\n", source, destination);
    
    int sockfd;
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    }
    
    // create a test data packet and 
    //push it to source router for transmitting to destinationrouter
    struct Packet sndPacket;
    sndPacket.type = DATAPACKET;
    sndPacket.sourceRouter = source;    
    sndPacket.destinationRouter = destination;
    sendPacket(source, &sndPacket, sockfd);
    
}