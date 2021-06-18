/*
 * Program 4 - Socket Programming
 * Majority of code from Class Lecture Example. Establish a multi-threaded
 * server-client program communicating with sockets using TCP/IP.
 * 
 * This code creates the server that provides a separate thread for every client
 * connection that is accepted.
 * Byron Kim
 * CSS 503b Dimpsey
 * 6/12/2021
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>    // pthread
#include <poll.h>       // poll
using namespace std;

const int BUFFSIZE = 1500;
const int NUM_CONNECTIONS = 5;

struct pollfd fds;
int timeout_msecs = 100;
int ret;

struct client_runner_struct {
    int newSD;
};

void* client_runner(void *param);


int main(int argc, char *argv[])
{
    /*
     * Argument Validation
     */
    if (argc < 2)
    {
        cerr << "usage: server port";
        return -1;
    }

    int serverPort = atoi(argv[1]);
    char databuf[BUFFSIZE];
    bzero(databuf, BUFFSIZE);

    /* 
     * Build address
     */
    struct sockaddr_in acceptSocketAddress;
    bzero(&acceptSocketAddress, sizeof(acceptSocketAddress));
    acceptSocketAddress.sin_family = AF_INET;
    acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    acceptSocketAddress.sin_port = htons(serverPort);

    /*
     *  Open socket and bind
     */
    int serverSD = socket(AF_INET, SOCK_STREAM, 0);
    const int on = 1;
    setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
    cout << "Socket #: " << serverSD << endl;
   
    int rc = bind(serverSD, (sockaddr *)&acceptSocketAddress, sizeof(acceptSocketAddress));
    if (rc < 0)
    {
        cerr << "Bind Failed" << endl;
    }
  
    /*
     *  listen and accept
     */
    listen(serverSD, NUM_CONNECTIONS);       //setting number of pending connections. 5 is plenty.
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    
    // Server operation 24/7. Process does not end until heat death of universe.
    while(true)
    {
        int newSD = accept(serverSD, (sockaddr *) &newSockAddr, &newSockAddrSize);
        cout << "Accepted Socket #: " << newSD <<endl;
        
        // Establish struct to update pthread parameters
        struct client_runner_struct client_threads;
        client_threads.newSD = newSD;

        // Create pThread for accepted connection
        // TODO For future multithread, modify pthread_t tid -> tids[]
        // I don't know how to test for multithread with school VM.
        pthread_t tid;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        
        pthread_create(&tid, &attr, client_runner, &client_threads);

        // TODO For future multithread, wait each pthread exit in array.
        // for (int i = 0; i < NUM_CONNECTIONS; i++) {
            pthread_join(tid, NULL);
        //}

        close(newSD);
        cout << "Socket " << newSD << " closed.\n" << endl;
    }
    
    close(serverSD);

    return 0;
}

void *client_runner(void *param)
{
    // Use struct as parameter to get multiple fields
    struct client_runner_struct *param_struct = (struct client_runner_struct*) param;
    int newSD = param_struct->newSD;
    cout << " Client socket ID = " << newSD << endl;


    char buffer[BUFFSIZE];
    // Receive from Client the number of repetitions
    read(newSD, buffer, BUFFSIZE);
    cout << "Client Repetitions: " << buffer << endl;
    bzero(buffer, BUFFSIZE);
   
    int buf_read = -1;
    int read_count = 0;
    // Run server-side READ() until buf_read reaches BUFFSIZE bytes of data, or 
    // Client runs shutdown(fd, SHUT_WR ). Meanwhile, when read() reads empty
    // buffer, read() will block until more written data is entered by Client
    // or EOF signal from Client.
    while( buf_read !=0 )
    {
        buf_read = read(newSD, buffer, BUFFSIZE);
        read_count++;
    }

    bzero(buffer, BUFFSIZE);
    sprintf(buffer, "%d", read_count);
    write(newSD, buffer, BUFFSIZE);

    pthread_exit(0);

    return NULL;
}