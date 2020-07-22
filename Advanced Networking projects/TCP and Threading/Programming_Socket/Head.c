
/*
@FIle: Head.c
@Author: Andrick Adhikari
@Description: 
Main method establishes server socket and waits for NODE_NO of nodes to connect.
Once all nodes are connected then work is divided among the node and thread handles these parallely.

 It took 84 seconds for serial work 
 It took 40 seconds for parallel work 
 */
#include <stdio.h>
#include "duSocket.h"
#include <omp.h>
#include <time.h>

int main() {
//Variables holding no of nodes and array for sockets
// Start and end of the range.
    const int NODE_NO = 4;
    const int start = 1000;
    const int end = 1000000;
    int newsockfd[NODE_NO];
    int result = 0;    
  
    
    int jobSize = (end - start)/NODE_NO;
    omp_set_num_threads(NODE_NO);

  // Get a socket of the right type
    int sockfd = setupServerSocket(7000);
    // set it up to listen
    
    for (int i = 0 ; i < NODE_NO ; i++)
    {
        newsockfd[i] = serverSocketAccept(sockfd);
    }
    printf("All nodes connected \n");

// stop accepting any more nodes once required no of nodes are connected
    close(sockfd);

//Start timer
    time_t beginTime = time(NULL);
    
  //Handle all the nodes in parallel with threads
    #pragma omp parallel for reduction(+:result)
    for (int i = 0 ; i < NODE_NO ; i++)
    {        
        int min;
        int max = jobSize * (i+1) + start;
        if (i == 0)
        {
            min = start;
        }
        else
        {
            min = jobSize * i + start + 1;
        }

        printf("\nNode %d started work with :%d  %d\n",i+1, min, max);
        writeInt(min,newsockfd[i]);
        writeInt(max,newsockfd[i]);
        result += readInt(newsockfd[i]);
        printf("Node %d is done \n", i+1);

//Close connection once the node is done
        close(newsockfd[i]);
    }
    
//Timer end once all the nodes are done
    time_t endTime = time(NULL);
    printf("\nResult is %d\nIt took %d seconds\n", result, (int)(endTime - beginTime));
    
    return 0;
}