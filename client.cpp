/* 
 * Program 4 - Socket Programming
 * Majority of code from Class Lecture Example. Changes by BK commented with double backslash
 * Establish a multi-threaded server-client program communicating with sockets using TCP/IP.
 * 
 * This code creates a client to connect with a pre-determined server.
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
#include <chrono>      // chrono::steady_clock::now()
using namespace std;

const int BUFFSIZE = 1500;

int main(int argc, char *argv[])
{
    /*
     *  Argument validation
     */
    if (argc < 7)
    {
        cerr << "usage: client serverName port repetition nbufs bufsize type";
        return -1;
    }

    /*
     * Initalization from program arguments
     */
    char *serverName = argv[1];   // Server Name for Byron:  10.102.26.45
    char *serverPort = argv[2];   // Port uses student's last 5 digits:  00054
    char *repetition = argv[3];
    int nbufs = atoi(argv[4]);
    int bufsize = atoi(argv[5]);
    int type = atoi(argv[6]);

    if (type <= 0 || type >= 4)
    {
        cerr << "Type: Incorrect Input. Enter 1, 2, or 3." << endl;
        return -1;
    }
    
    char databuf[nbufs][bufsize];
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int clientSD = -1;

    /*
     * Use getaddrinfo() to get addrinfo structure corresponding to serverName / Port
	 * This addrinfo structure has internet address which can be used to create a socket too
     */
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;					/* Allow IPv4 or IPv6*/
    hints.ai_socktype = SOCK_STREAM;					/* TCP */
    hints.ai_flags = 0;							/* Optional Options*/
    hints.ai_protocol = 0;						/* Allow any protocol*/
    int rc = getaddrinfo(serverName, serverPort, &hints, &result);
    if (rc != 0)
    {
       cerr << "ERROR: " << gai_strerror(rc) << endl;
       exit(EXIT_FAILURE);
    }

    /*
     * Iterate through addresses and connect
     */
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        clientSD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (clientSD == -1)
		{
            continue;
        }
		/*
		* A socket has been successfully created
		*/
        rc = connect(clientSD, rp->ai_addr, rp->ai_addrlen);
        if (rc < 0)
        {
            cerr << "Connection Failed" << endl;
            close(clientSD);
            return -1;
        }
        else	//success
        {
            break;
        }
    }

    if (rp == NULL)
    {
        cerr << "No valid address" << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "Client Socket: " << clientSD << endl;
    }
    freeaddrinfo(result);

    // Send Message to Server letting it know the number of iterations of the
    // test it will perform  
    write(clientSD, repetition, sizeof(repetition));
    
    // Start Chrono time
    auto start = chrono::steady_clock::now();

    // Perform Task with Server
    for (int i = 0; i < atoi(repetition); i++)
    {
        if (type == 1)
        {
            // Multiple write()
            for (int j = 0; j < nbufs; j++)
            {
                write(clientSD, databuf[j], bufsize);
            }
        }
        else if (type == 2)
        {
            // writev()
            struct iovec vector[nbufs];
            for (int j = 0; j < nbufs; j++)
            {
                vector[j].iov_base = databuf[j];
                vector[j].iov_len = bufsize;
            }
            writev(clientSD, vector, nbufs);
        }
        else // type == 3
        {
            // One massive write()
            write(clientSD, databuf, nbufs * bufsize);
        }
    }
    // End of Task, block writing on Client side so Server read() returns EOF instead of blocking
    if (shutdown(clientSD, SHUT_WR) < 0)
    {
        cerr << "Client socket write shutdown failed" << endl;
        return -1;
    }
    
    // End Chrono time.
    auto end = chrono::steady_clock::now();
    chrono::duration<double, micro> elapsed_time = end-start;

    // Receive #reads from server
    read(clientSD, databuf[0], BUFFSIZE);

    // Summary
    int total_data = nbufs * bufsize * atoi(repetition);
    double bandwidth = total_data / elapsed_time.count();
    cout << "Test " << type << ": time = " << elapsed_time.count() 
        << " usec, #reads = " << databuf[0] << ", throughput " << bandwidth
        << " Gbps" << endl;

    close(clientSD);
    return 0;
}