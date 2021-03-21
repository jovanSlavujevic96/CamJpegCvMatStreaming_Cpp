/* Receiver/client multicast Datagram example. */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <opencv2/opencv.hpp>

#include <cstdint>

#include "utils.h"

int main(int argc, char **argv)
{
    /* socket variables */
    struct sockaddr_in localSock;
    struct ip_mreq group;
    int sd = -1;

    /* OpenCV variables */
    cv::Mat img;
    std::vector<uchar> streaming_frame;

    /* Create a datagram socket on which to receive. */
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd < 0)
    {
        perror("Opening datagram socket error");
        exit(1);
    }
    else
        printf("Opening datagram socket....OK.\n");

    /* Enable SO_REUSEADDR to allow multiple instances of this */
    /* application to receive copies of the multicast datagrams. */
    {
        int32_t reuse = 1;
        if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
        {
            perror("Setting SO_REUSEADDR error");
            close(sd);
            exit(1);
        }
        else
        {
            printf("Setting SO_REUSEADDR...OK.\n");
        }
    }

    /* Bind to the proper port number with the IP address */
    /* specified as INADDR_ANY. */
    memset(&localSock, 0, sizeof(localSock));
    localSock.sin_family = AF_INET;
    localSock.sin_port = htons(PORT);
    localSock.sin_addr.s_addr = INADDR_ANY;

    if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock)))
    {
        perror("Binding datagram socket error");
        close(sd);
        exit(1);
    }
    else
    {
        printf("Binding datagram socket...OK.\n");
    }

    /* Join the multicast group 226.1.1.1 on the local 203.106.93.94 */
    /* interface. Note that this IP_ADD_MEMBERSHIP option must be */
    /* called for each local interface over which the multicast */
    /* datagrams are to be received. */
    group.imr_multiaddr.s_addr = inet_addr(MULTICAST_IPV4);
    group.imr_interface.s_addr = INADDR_ANY;
    if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
    {
        perror("Adding multicast group error");
        close(sd);
        exit(1);
    }
    else
    {
        printf("Adding multicast group...OK.\n");
    }
    
    int bytes;
    while(cv::waitKey(1) != 27)
    {
        /* set size to max udp payload size */
        streaming_frame.resize(MAX_UDP_PAYLOAD_SIZE);
        /* get frame from streaming server */
        if ((bytes = read(sd, streaming_frame.data(), MAX_UDP_PAYLOAD_SIZE)) < 0)
        {
            perror("Receiving datagram message error");
            break;
        }
        /* resize vector and decode frame to get cv::Mat */
        streaming_frame.resize(bytes);
        if((img = cv::imdecode(std::move(streaming_frame), cv::IMREAD_UNCHANGED)).empty())
        {
            break;
        }
        /* show image */
        cv::imshow("received", img);
    }
    close(sd);
    return 0;
}
