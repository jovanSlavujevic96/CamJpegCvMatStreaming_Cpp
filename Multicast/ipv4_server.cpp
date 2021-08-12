/* Send Multicast Datagram code example. */

#include "utils.h"

#include <memory.h>
#include <string.h>
#include <cstdint>
#include <vector>

#include <opencv2/opencv.hpp>

int main (int argc, char** argv)
{
    struct in_addr localInterface;
    struct sockaddr_in groupSock;
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
    if( (sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
#else
    if ((sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) <= INVALID_SOCKET)
#endif
    {
        _PrintSocketError("Opening datagram socket error");
        return 1;
    } 
    else
    {
        printf("Opening the datagram socket...OK.\n");
    }

    /* Enable SO_REUSEADDR to allow multiple instances of this */
    /* application to receive copies of the multicast datagrams. */
    {
        int32_t reuse = 1;
        if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
        {
            _PrintSocketError("Setting SO_REUSEADDR error");
            _CloseSocket(sd);
            exit(1);
        }
        else
        {
            printf("Setting SO_REUSEADDR...OK.\n");
        }
    }

    /* Initialize the group sockaddr structure with a */
    memset(&groupSock, 0, sizeof(groupSock));
    groupSock.sin_family = AF_INET;
    groupSock.sin_addr.s_addr = INADDR_ANY;
    groupSock.sin_port = htons(PORT);

    /* Set local interface for outbound multicast datagrams. */
    /* The IP address specified must be associated with a local, */
    /* multicast capable interface. */
    memset(&localInterface, 0, sizeof(localInterface));
    inet_pton(AF_INET, MULTICAST_IPV4, &groupSock.sin_addr);
    if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
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
    return 0;
}
