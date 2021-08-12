/* Send Multicast Datagram code example. */

#include "utils.h"

#include <memory.h>
#include <string.h>
#include <cstdint>
#include <vector>
#include <iostream>

#include <opencv2/opencv.hpp>

int main(int argc, char const *argv[]) 
{
    /* socket variables */
    struct in6_addr localInterface;
    struct sockaddr_in6 groupSock;
    SOCKET sd = INVALID_SOCKET;

    /** OpenCV variables */
    cv::VideoCapture cap;
    cv::Mat frame;
    const std::vector<int> param = {cv::IMWRITE_JPEG_QUALITY, 90};
    std::vector<uchar> streaming_buffer;

    /* try to open default camera */
    if(!cap.open(0, cv::CAP_DSHOW))
    {
        return 1;
    }

#ifdef WIN
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(1, 1), &wsd) != 0)
    {
        printf("WSAStartup failed\n");
        return -1;
}
#endif //WIN

    /* Create a datagram socket on which to send/receive. */
#ifdef WIN
        if ((sd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
#else
        if ((sd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) <= INVALID_SOCKET)
#endif
    {
        _PrintSocketError("Opening datagram socket error");
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
        if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof reuse) < 0) 
        {
            _PrintSocketError("Setting SO_REUSEADDR error");
            _CloseSocket(sd);
            return 1;
        } 
        else 
        {
            std::cout << "Setting SO_REUSEADDR...OK." << std::endl;
        }
    }

    /* Initialize the group sockaddr structure with a */
    memset(&groupSock, 0, sizeof(groupSock));
    groupSock.sin6_family = AF_INET6;
    inet_pton(AF_INET6, MULTICAST_IPv6, &groupSock.sin6_addr);
    groupSock.sin6_port = htons(PORT);

    /* Set local interface for outbound multicast datagrams. */
    /* The IP address specified must be associated with a local, */
    /* multicast capable interface. */
    memset(&localInterface, 0, sizeof(localInterface));
    localInterface = in6addr_any;
    if(setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
    {
        _PrintSocketError("Setting local interface error");
        _CloseSocket(sd);
        exit(1);
    }
    else
    {
        printf("Setting the local interface...OK\n");
    }

    while(true)
    {
        /* read frame from camera */
        cap >> frame;
        if(frame.empty() )
        {
            break;
        }
        else if(!cv::imencode(".jpg", frame, streaming_buffer, param) ) /* encode/compress frame */
        {
            break;
        }
        else if(sendto(sd, (const char*)streaming_buffer.data(), (int)streaming_buffer.size(), 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0) /* send compressed package */ 
        {
            _PrintSocketError("Sending datagram message error");
            break;
        }
    }
    _CloseSocket(sd);
    cap.release();
    return 0;
}
