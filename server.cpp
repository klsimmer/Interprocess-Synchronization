//
//  main.cpp
//  PA2Server
//
//  Created by Kaytlin Simmer
//

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

const int base = 10;
const int segments = 7;

const int segmentDisplay[base][segments] =
{
  {1,1,1,1,1,1,0},
  {0,1,1,0,0,0,0},
  {1,1,0,1,1,0,1},
  {1,1,1,1,0,0,1},
  {0,1,1,0,0,1,1},
  {1,0,1,1,0,1,1},
  {1,0,1,1,1,1,1},
  {1,1,1,0,0,0,0},
  {1,1,1,1,1,1,1},
  {1,1,1,1,0,1,1}
};

//fireman function from class
void fireman(int)
{
   while (waitpid(-1, NULL, WNOHANG) > 0)
       std::cout << "A child process ended" << std::endl;
}

int main(int argc, char *argv[])
{
    //code from lecture 9/30
     int sockfd, newsockfd, portno, clilen;
    
     char buffer[256];
    
     struct sockaddr_in serv_addr, cli_addr;
     int n;
    //not recieving ip address/host name. only recieve port #
     if (argc < 2)
     {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
    
    //create socket, same as client, only for receiving connection
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
        {
            fprintf(stderr,"ERROR opening socket\n");
            exit(1);
        }
    
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
    //address is now inaddr_any, can recieve requests from any internet addresses that are avail
     serv_addr.sin_addr.s_addr = INADDR_ANY;
    //assign port
     serv_addr.sin_port = htons(portno);
    
    //not connecting - bind info from sock addr
    //setting all params to recieve info from all poss adr
    bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
     /*{
         fprintf(stderr,"ERROR on binding\n");
         exit(1);
     }*/
     
    //# of max connections you can recieve at a particular time
     listen(sockfd,5);
    
    //client address
     clilen = sizeof(cli_addr);
    
    //infinite loop
    //fireman function from class
    signal(SIGCHLD, fireman);
    while (true)
    {
    //every time u recieve a req u create a new socket
     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen);
    if (fork() == 0)
    {
        if (newsockfd < 0)
        {
            fprintf(stderr,"ERROR on accept\n");
            exit(1);
        }
        
        bzero(buffer,256);
        int value = 0;
        //child process read
        n = read(newsockfd, &value, sizeof(int));
        if (n < 0)
        {
            fprintf(stderr,"ERROR reading from socket\n");
            exit(1);
        }
        
        //print value
        //send message that you recieve on client side
        printf("Value = %d\n",value);
        std::string s;
        s+=std::to_string(value);
        s+=" = ";
        for (int i = 0; i < segments; i++)
        {
            s+=std::to_string(segmentDisplay[value][i]);
            if (i < segments)
                s+=" ";

        }
        std::cout<<s<<std::endl;
        int x = s.length();
        char buf[x];
        for (int i = 0; i < x; i++)
            buf[i] = s[i];
        n = write(newsockfd,buf,255);
        if (n < 0)
        {
            fprintf(stderr,"ERROR reading from socket\n");
            exit(1);
        }
        close(newsockfd);
        _exit(0);
    }
    }
    //wait for child process to end
    wait(0);
    //close socket for server
    close(sockfd);
     
     return 0;
}
