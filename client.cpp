//
//  main.cpp
//  PA2Client
//
//  Created by Kaytlin Simmer
//

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <vector>
#include <pthread.h>

/*struct info
{
    int* digit = new int;
};*/

//struct for data of the child threads
struct childThreadData
{
    struct hostent* CTDserver;
    int CTDport;
    long int CTDlineVal;
    int CTDindex;
    //for printing from grandchild thread to main
    std::vector<std::string> outputs;
   // info* CTDinfo = new info;
};

//struct for data of the grandchild threads
struct grandChildThreadData
{
    struct hostent* GCTDserver;
    int GCTDport;
    long int GCTDlineVal; //currently using to hold number of digits
    int GCTDindex;
    int GCTDdigit;
    // //for printing from grandchild thread to main
    std::string gcoutput;
    //info* GCTDinfo = new info;
};

//grandchild thread function which creates the socket and writes to  the server
void* grandChildThread(void* y)
{
    struct grandChildThreadData* GCTptr = (struct grandChildThreadData*) y;

    //creating the socket
    int sockfd, n;
    struct sockaddr_in serv_addr;
    //creating the buffer to write into
    char buffer[256];
    
    //create socket, (family (internet), type of socket, 0)
    //manipulated code from lecture on 9/30
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        fprintf(stderr,"ERROR opening socket\n");

    //zero all info in struct server address
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    //attain IP address from server struct
    serv_addr.sin_family = AF_INET;
    bcopy( (char *) GCTptr->GCTDserver->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         GCTptr->GCTDserver->h_length);
    
    //copy port # we're receiving --> going to use to connect to server
    serv_addr.sin_port = htons(GCTptr->GCTDport);
    
    //checking that it connects
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        fprintf(stderr,"ERROR connecting\n");

    //create a buffer, an array u initalize w 0s
   bzero(buffer,256);
//   fgets(buffer,255,stdin);
    
    // write to the server
    n = write(sockfd, &GCTptr->GCTDdigit, sizeof(int));
    //check that it wrote
    if  (n < 0)
    {
        fprintf(stderr,"ERROR writing to socket");
    }
    
    // read reply from the server
    bzero(buffer,256);
    n = read(sockfd, buffer, 255);
    //store the value of the buffer into the grandchild thread's buffer so it can print it

    //check that it read something
    if  (n < 0)
    {
        fprintf(stderr,"ERROR reading from socket");
    }
    std::string s(buffer);
    GCTptr->gcoutput = s;
    
   printf("%s\n", buffer);
    
    //close the socket
    close(sockfd);
    
    //clear the  buffer
    bzero(buffer,256);
    return nullptr;
}

//child thread function  that creates the grandchild threads
void* childThread(void* x)
{
    struct childThreadData* CTptr = (struct childThreadData*) x;
    
    //the number at the line
    long valAtI = CTptr->CTDlineVal;
    
    //vector of ints to hold each digit of the number
    std::vector <int> digitsOfVal;
    
    //attaining each digit seperately
    int count = 0;
    while (valAtI > 0)
    {
        digitsOfVal.push_back(valAtI%10);
        count++;
        valAtI/=10;
    }
    
    //creating N grandchild threads for N number of digits
    pthread_t* grandChildT = new pthread_t[count];
    //holds  the data of each thread
    static struct grandChildThreadData* grandChildVal = new grandChildThreadData[count];
    
    //creates the grandchild thread and assigns them their data
    for (int j = 0; j < count; j++)
    {
        //server, will be the same as the child thread's
        grandChildVal[j].GCTDserver = CTptr->CTDserver;
        //port, will be the same as the child  thread's
        grandChildVal[j].GCTDport = CTptr->CTDport;
        //the  complete number
        grandChildVal[j].GCTDlineVal = count;
        //the grand child  thread's number
        grandChildVal[j].GCTDindex = j;
        //the digit we're dealing with
        grandChildVal[j].GCTDdigit = digitsOfVal.at(j);
        
        //check that the thread was created and call the grand child thread function
        if (pthread_create(&grandChildT[j],nullptr,grandChildThread, &grandChildVal[j]))
        {
            fprintf(stderr,"Error creating thread\n");
          //  return 1;
        }
        
    }
    
    //rejoin all  the grandchild threads
    for (int k = 0; k < count; k++)
    {
        
        if (pthread_join(grandChildT[k], NULL))
        {
            fprintf(stderr, "Error joing threads\n");
            //return 2;
        }
        CTptr->outputs.push_back(grandChildVal[k].gcoutput);
    }

    return nullptr;
}

int main(int argc, char *argv[])
{
    //code from lecture on 9/30
    //port  number
    int portno;
    //server
    struct hostent *server;
    //for input, temp holds each val
    long nums;
    //vector  for all the values of the file
    std::vector <long> values;
  
    //get host name
    if (argc < 3)
    {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    //3rd arg from cmd line is port number --> recieve arg from cmd line, recieve port number
    portno = atoi(argv[2]);
    //get host char array- name of server u want to commumicate with
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr,"ERROR, invalid host\n");
        exit(0);
    }
    
    //end  w ctr+D
    //getting all the nums  from the file
    while (std::cin>>nums)
    {
        //put the values into the vector of all the nums
        values.push_back(nums);
    }
    
    //attain N number of child threads we need to create by size of the vector, N number of values
   long numOfChildThreads = values.size();
    
    //Create child threads
    //array of child threads
    pthread_t* childT = new pthread_t[numOfChildThreads];
    //struct for the data of each thread
    static struct childThreadData* childVal = new childThreadData[numOfChildThreads];
    for (int i = 0; i < numOfChildThreads; i++)
    {
        //server and port
        childVal[i].CTDserver = server;
        childVal[i].CTDport = portno;
        //value of the entire line
        childVal[i].CTDlineVal = values.at(i);
        //the child thread's number
        childVal[i].CTDindex = i;
        //check  if thread is created, call child  thread  function
        if (pthread_create(&childT[i],nullptr,childThread, &childVal[i]))
        {
            fprintf(stderr,"Error creating thread\n");
            return 1;
        }
        
    }
    
    //rejoin the child threads
    for (int i = 0; i < numOfChildThreads; i++)
    {
        if (pthread_join(childT[i], nullptr))
        {
            fprintf(stderr, "Error joing threads\n");
            return 2;
        }
        std::cout<<childVal[i].outputs.at(i);
    }
    
    
    return 0;
    
}

