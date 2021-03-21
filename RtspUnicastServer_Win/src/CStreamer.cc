// MediaLAN 02/2013
// CRtspSession
// - JPEG packetizer and UDP/TCP based streaming

#include <cstdio>

#include "CSampleModel.h"
#include "CStreamer.h"

#define KRtpHeaderSize 12           // size of the RTP header
#define KJpegHeaderSize 8           // size of the special JPEG payload header

CStreamer::CStreamer(SOCKET aClient) :
    m_Client{ aClient },
    RtpBuf{ 0 }
{
    m_RtpSocket = INVALID_SOCKET;
    m_RtcpSocket = INVALID_SOCKET;

    m_RtpServerPort  = 0;
    m_RtcpServerPort = 0;
    m_RtpClientPort  = 0;
    m_RtcpClientPort = 0;

    m_SequenceNumber = 0;
    m_Timestamp      = 0;
    m_SendIdx        = 0;
    m_TCPTransport   = false;
}

CStreamer::~CStreamer()
{
    closesocket(m_RtpSocket);
    closesocket(m_RtcpSocket);
}

void CStreamer::InitTransport(u_short aRtpPort, u_short aRtcpPort, bool TCP)
{
    sockaddr_in Server;  

    m_RtpClientPort  = aRtpPort;
    m_RtcpClientPort = aRtcpPort;
    m_TCPTransport   = TCP;

    if (!m_TCPTransport)
    {   // allocate port pairs for RTP/RTCP ports in UDP transport mode
        Server.sin_family      = AF_INET;   
        Server.sin_addr.s_addr = INADDR_ANY;   
        for (u_short P = 6970; P < 0xFFFE ; P += 2)
        {
            m_RtpSocket     = socket(AF_INET, SOCK_DGRAM, 0);                     
            Server.sin_port = htons(P);
            if (bind(m_RtpSocket,(sockaddr*)&Server,sizeof(Server)) == 0)
            {   // Rtp socket was bound successfully. Lets try to bind the consecutive Rtsp socket
                m_RtcpSocket = socket(AF_INET, SOCK_DGRAM, 0);
                Server.sin_port = htons(P + 1);
                if (bind(m_RtcpSocket,(sockaddr*)&Server,sizeof(Server)) == 0) 
                {
                    m_RtpServerPort  = P;
                    m_RtcpServerPort = P+1;
                    break; 
                }
                else
                {
                    closesocket(m_RtpSocket);
                    closesocket(m_RtcpSocket);
                }
            }
            else
            {
                closesocket(m_RtpSocket);
            }
        }
    }

    // get client address for UDP transport
    getpeername(m_Client, (struct sockaddr*)&RecvAddr, &RecvLen);
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(m_RtpClientPort);
}

u_short CStreamer::GetRtpServerPort()
{
    return m_RtpServerPort;
}

u_short CStreamer::GetRtcpServerPort()
{
    return m_RtcpServerPort;
}

void CStreamer::StreamImage(CSampleModel* streamingSample)
{
    const int16_t RtpPacketSize = streamingSample->getImageSize() + KRtpHeaderSize + KJpegHeaderSize;

    // Prepare the first 4 byte of the packet. This is the Rtp over Rtsp header in case of TCP based transport
    RtpBuf[0] = '$';            // magic number
    RtpBuf[1] = 0x00u;          // number of multiplexed subchannel on RTPS connection - here the RTP channel
    RtpBuf[2] = (unsigned char)((RtpPacketSize & 0x0000FF00) >> 8);
    RtpBuf[3] = (unsigned char)(RtpPacketSize & 0x000000FF);
    //Prepare the 12 byte RTP header
    RtpBuf[4] = 0x80u;                                  // RTP version
    RtpBuf[5] = 0x9au;                                  // JPEG payload (26) and marker bit
    RtpBuf[7] = m_SequenceNumber & 0x0FF;               // each packet is counted with a sequence counter
    RtpBuf[6] = m_SequenceNumber >> 8;
    RtpBuf[8] = (unsigned char)((m_Timestamp & 0xFF000000) >> 24);   // each image gets a timestamp
    RtpBuf[9] = (unsigned char)((m_Timestamp & 0x00FF0000) >> 16);
    RtpBuf[10] = (unsigned char)((m_Timestamp & 0x0000FF00) >> 8);
    RtpBuf[11] = (unsigned char)(m_Timestamp & 0x000000FF);
    RtpBuf[12] = 0x13u;                                 // 4 byte SSRC (sychronization source identifier)
    RtpBuf[13] = 0xf9u;                                 // we just an arbitrary number here to keep it simple
    RtpBuf[14] = 0x7eu;
    RtpBuf[15] = 0x67u;
    // Prepare the 8 byte payload JPEG header
    RtpBuf[16] = 0x00u;                                 // type specific
    RtpBuf[17] = 0x00u;                                 // 3 byte fragmentation offset for fragmented images
    RtpBuf[18] = 0x00u;
    RtpBuf[19] = 0x00u;
    RtpBuf[20] = 0x01u;                                 // type
    RtpBuf[21] = 0;//0x5eu;                                 // quality scale factor
    RtpBuf[22] = streamingSample->getImageWidth()/8;    // width  / 8 -> 48 pixel
    RtpBuf[23] = streamingSample->getImageHeight()/8;   // height / 8 -> 32 pixel
    
    // append the JPEG scan data to the RTP buffer
    std::vector<uint8_t>& StreamingImage = streamingSample->getImageVector();
    StreamingImage.insert(StreamingImage.begin(), RtpBuf, RtpBuf+RTSP_HEADER_SIZE);

    m_SequenceNumber++;                              // prepare the packet counter for the next packet
    m_Timestamp += 3600;                             // fixed timestamp increment for a frame rate of 25fps

    if (m_TCPTransport)
    { // RTP over RTSP - we send the buffer + 4 byte additional header
        send(m_Client, (char*)StreamingImage.data(), RtpPacketSize + 4, 0);
    }
    else
    { // UDP - we send just the buffer by skipping the 4 byte RTP over RTSP header
        sendto(m_RtpSocket, (char*)&StreamingImage.data()[4], RtpPacketSize, 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
    }
}
