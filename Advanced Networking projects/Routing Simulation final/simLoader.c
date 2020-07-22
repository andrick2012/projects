#include"simHeader.h"
#include <omp.h>

int main(int argc, char **argv)
{
    //Check for correct number of arguments
    if(argc != 3)
    {
        fprintf(stderr,"Usage: ./loader <Number of Router> <Configuration file>\n");
        exit(0);
    }
    // total number of routers
    int routerNumber = atoi(argv[1]);
    int distanceMatrix[routerNumber][routerNumber]; // distance vector for all the routers
    
    FILE * fp;
    FILE * fp2;
    char * line = NULL;
    size_t len = 0;
    
    // read the root graph file
    fp = fopen(argv[2], "r");
    if (fp == NULL)
    {
        fprintf(stderr,"Root File: doesn't exist\n");
        exit(0);   
    }
    
    /*
     * following section reads all the graph files in the root graph files
     * if required number of graph files or values are not found or read.
     * Then program is terminated. 
     *
     */
    int iRouter = 0;
    while ((getline(&line, &len, fp)) != -1) {
        if(line[strlen(line) -1 ] == '\n')
            line[strlen(line) -1 ] = '\0';
        printf("\nReading: %s \n", line);
        fp2 = fopen(line, "r");
        if(fp2 == NULL)
        {
            fprintf(stderr,"Graph File Router %d: doesn't exist\n", iRouter);
            fclose(fp2);
            fclose(fp);
            exit(0);
        }
        int iTableIndex = 0; 
        do 
        { 
          fscanf (fp2, "%d", &distanceMatrix[iRouter][iTableIndex]);
          printf ("%d ", distanceMatrix[iRouter][iTableIndex]);
          iTableIndex++;
          
        }while(!feof (fp2));
      
        printf("\n");
        fclose(fp2);
        //terminate if graph informations are lacking
        if(iTableIndex != routerNumber)
        {
            fprintf(stderr,"Graph file for %d is incomplete\n", iRouter);
            fclose(fp);
            exit(0);
        }
        iRouter++;
    }
    fclose(fp);
    //terminate if graph files for all the routers are not present
     if(iRouter != routerNumber)
    {
        fprintf(stderr,"Not enough graph files provided\n");
        exit(0);
    }
    
    // print all the DV tables
    printTables((int *)distanceMatrix, routerNumber);
    
    omp_set_num_threads(routerNumber);
    // Fire up a thread for each of the router
    #pragma omp parallel for 
    for (int i = 0 ; i < routerNumber ; i++)
    {    
        printf("\nRouter %d started with port no %d \n",i+1, BASEPORT+i);
        int sockfd = setupServerSocket(BASEPORT + i); // socket for a router      
        
         /*
          * viaMatrix are important for identifying next hop if direct route doesnt exist.
          * 
          * -1 identifies a direct route.
          * otherwise next hop router value is stored.
          */
        int *viaMatrix = malloc(routerNumber * sizeof(int));
        memset(viaMatrix, -1, routerNumber * sizeof(int));
        
        /*
         *send initial control packets to all the routers from each router
         */
        sendDVUpdate(sockfd, i,(int *)&distanceMatrix[i], routerNumber, viaMatrix);
        
        /*
         * for receiving packets from other routers or test program
         */
        struct sockaddr_in cliaddr;
        memset(&cliaddr, 0, sizeof(cliaddr));        
        unsigned int len = sizeof(cliaddr);         
        struct Packet recvPacket; 
        
        //loop to continously recieve packets
        while(recvfrom(sockfd, &recvPacket, sizeof(recvPacket), 
		 0, (struct sockaddr *) &cliaddr, &len))
        {
            if(recvPacket.type == DATAPACKET)
                printf("\n");
            #pragma omp critical
            printf("\nRouter %d recieved packet :\n", i);
            // if control packet and meant for this router
            if(recvPacket.type == DVPACKET && recvPacket.destinationRouter == i)
            {
                #pragma omp critical
                printf("\nDV update packet from Router %d recieved :\n", recvPacket.sourceRouter);
                //check for necessary update
                if(updateDVtable((int *)&distanceMatrix[i],&recvPacket, routerNumber, viaMatrix) == 1)
                {
                    // if table updated, send the table to all the router
                    #pragma omp critical
                    printTable((int *)distanceMatrix[i], routerNumber, i);
                    sendDVUpdate(sockfd, i,(int *)&distanceMatrix[i], routerNumber, viaMatrix);                    
                }
                else
                {
                    #pragma omp critical
                    printf("No update required\n");
                }
            }
            //if data packet and destination router then print total cost of the transmission.
            else if(recvPacket.type == DATAPACKET && recvPacket.destinationRouter == i)
            {
                #pragma omp critical
                printf("Data Packet reached the destination Router, Total cost : %d\n", distanceMatrix[i][recvPacket.sourceRouter]);
            }
            else
            {
                // Don't forward packets if the router doesn't exist or link to the router doesn't exist
                if(distanceMatrix[i][recvPacket.destinationRouter] == -1 || recvPacket.destinationRouter >= routerNumber)
                {
                    #pragma omp critical
                    printf("\nPacket cannot be sent to Router: %d -> No link\n",recvPacket.destinationRouter);
                }
                else
                {                
                    // Directly forward if viaMatrix has -1 for the destination
                    if(viaMatrix[recvPacket.destinationRouter] == -1)
                    {
                        #pragma omp critical
                        printf("Packet forwarded from Router %d to -> %d Router : cost :%d \n"
                                ,i, recvPacket.destinationRouter,distanceMatrix[i][recvPacket.destinationRouter]);
                        sendPacket(recvPacket.destinationRouter, &recvPacket, sockfd);
                    }
                    // forward to next hop if indirect route present
                    else
                    {
                        int nextHop = viaMatrix[recvPacket.destinationRouter];
                        #pragma omp critical
                        printf("Packet forwarded from Router %d to -> %d Router : cost : %d\n",
                                i, nextHop,distanceMatrix[i][nextHop]);
                        sendPacket(nextHop, &recvPacket, sockfd); 
                    }
                       
                }                
            }
        }
        
    }
    
}


