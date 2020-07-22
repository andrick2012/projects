/*
File: Node.c
Author: Andrick Adhikari
Description:
1. main method calls the server and takes in min and max of the range, 
	the call prime and outputs its results to head.
2.Prime : calculates the number of prime number within min and max inclusive.
*/

#include <stdio.h>
#include "duSocket.h"
int prime(int min, int max)
{
    int numberPrime = 0;
    for(int i = min; i <= max; i++)
    {
        if (i < 2)
        {
            continue;
        }
        int flag = 0;
        for(int j = 2; j <= i/2; j++)
        {
            if(i%j == 0){
                flag = 1;
                break;
            }
        }
        if(flag != 1)
            numberPrime++;
    }
    return numberPrime;
}
int main() {

  // Socket pointer
   int sockfd = callServer("localhost", 7000);
   int min = readInt(sockfd);
   int max = readInt(sockfd);
   int value = prime(min, max);
   writeInt(value, sockfd);
   
   close(sockfd);
   return 0;

}
