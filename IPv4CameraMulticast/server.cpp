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

#include "utils.h"

int main (int argc, char** argv)
{
    struct in_addr localInterface;
    struct sockaddr_in groupSock;
    int sd = -1;
    
    /** OpenCV variables */
    cv::VideoCapture cap;
    cv::Mat frame;
    const std::vector<int> param = {cv::IMWRITE_JPEG_QUALITY, 90};
    std::vector<uchar> streaming_buffer;

    /* try to open default camera */
    if(!cap.open(0))
    {
        return 1;
    }

    /* Create a datagram socket on which to send/receive. */
    if((sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) 
    {
        perror("Opening datagram socket error");
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
            perror("Setting SO_REUSEADDR error");
            close(sd);
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
    localInterface.s_addr = INADDR_ANY;
    if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
    {
        perror("Setting local interface error");
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
        else if(sendto(sd, streaming_buffer.data(), streaming_buffer.size(), 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0) /* send compressed package */ 
        {
            perror("Sending datagram message error");
            break;
        }
    }
    close(sd);
    return 0;
}
