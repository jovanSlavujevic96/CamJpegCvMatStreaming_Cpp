/* Send Multicast Datagram code example. */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include <opencv2/opencv.hpp>

#include <cstdint>
#include <vector>
#include <iostream>

#include "utils.h"

int main(int argc, char const *argv[]) 
{
    /* socket variables */
    struct sockaddr_in6 localSock;
    struct ipv6_mreq group;
    int sd = -1;

    /* OpenCV variables */
    cv::Mat img;
    std::vector<uchar> streaming_frame;

    /* Create a datagram socket on which to send/receive. */
    if((sd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) < 0) 
    {
        perror("Opening datagram socket error");
        return 1;
    } 
    else 
    {
        std::cout << "Opening the datagram socket...OK.\n";
    }

    /* Enable SO_REUSEADDR to allow multiple instances of this */
    /* application to receive copies of the multicast datagrams. */
    {
        int32_t reuse = 1;
        if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
        {
            perror("Setting SO_REUSEADDR error");
            close(sd);
            return 1;
        } 
        else 
        {
            std::cout << "Setting SO_REUSEADDR...OK.\n";
        }
    }

    /* Initialize the group sockaddr structure with a */
    memset( &localSock, 0, sizeof(localSock));
    localSock.sin6_family = AF_INET6;
    localSock.sin6_addr = in6addr_any; // address of the group
    localSock.sin6_port = htons(PORT);

    if(bind(sd, (sockaddr*)&localSock, sizeof(localSock))) 
    {
        perror("Binding datagram socket error");
        close(sd);
        return 1;
    } 
    else 
    {
        std::cout << "Binding datagram socket...OK.\n";
    }

    /* Join the multicast group ff0e::/16 on the local  */
    /* interface. Note that this IP_ADD_MEMBERSHIP option must be */
    /* called for each local interface over which the multicast */
    /* datagrams are to be received. */
    inet_pton (AF_INET6, MULTICAST_IPv6 , &group.ipv6mr_multiaddr.s6_addr);
    group.ipv6mr_interface = *(unsigned int*)&in6addr_any;

    if(setsockopt(sd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &group, sizeof(group))) 
    {
        perror("Adding multicast group error");
        close(sd);
        return 1;
    } 
    else 
    {
        std::cout << "Setting the local interface...OK\n";
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
