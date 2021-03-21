// MediaLAN 02/2013
// CRtspSession
// - JPEG packetizer and UDP/TCP based streaming

#ifndef _CStreamer_H
#define _CStreamer_H

#include "utils.h"

#define RTSP_HEADER_SIZE 24

class CSampleModel;

class CStreamer
{
public:
    CStreamer(SOCKET aClient);
    ~CStreamer();

    void    InitTransport(u_short aRtpPort, u_short aRtcpPort, bool TCP);
    u_short GetRtpServerPort();
    u_short GetRtcpServerPort();
    void    StreamImage(CSampleModel* streamingSample);

private:
    SOCKET  m_RtpSocket;          // RTP socket for streaming RTP packets to client
    SOCKET  m_RtcpSocket;         // RTCP socket for sending/receiving RTCP packages

    sockaddr_in RecvAddr;
    int RecvLen = sizeof(RecvAddr);

    u_short m_RtpClientPort;      // RTP receiver port on client 
    u_short m_RtcpClientPort;     // RTCP receiver port on client
    u_short m_RtpServerPort;      // RTP sender port on server 
    u_short m_RtcpServerPort;     // RTCP sender port on server

    u_short     m_SequenceNumber;
    DWORD       m_Timestamp;
    int         m_SendIdx;
    bool        m_TCPTransport;
    SOCKET      m_Client;

    uint8_t    RtpBuf[RTSP_HEADER_SIZE];
};

#endif