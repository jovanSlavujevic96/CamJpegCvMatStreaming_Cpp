#include <iostream>
#include <chrono>
#include <thread>

#include "utils.h"
#include "Cstreamer.h"
#include "CRtspSession.h"
#include "CRtspMaster.h"
#include "CRtspClientHandler.h"

#define RCV_BUF_SIZE 1024 /* 1 kb = 1024 b */

CRtspClientHandler::CRtspClientHandler(SOCKET ClientSocket, CRtspMaster* rtspMaster) :
	m_ClientSocket{ ClientSocket },
    m_RtspMaster{ rtspMaster },
	m_WorkingThread{ std::thread(&CRtspClientHandler::RtspClientSessionHandling, this) }
{

}

CRtspClientHandler::~CRtspClientHandler()
{
    m_WorkingThread.detach();
    ::closesocket(m_ClientSocket);
}

SOCKET CRtspClientHandler::getSocket() const
{
    return m_ClientSocket;
}

void CRtspClientHandler::RtspClientSessionHandling()
{
    char         RecvBuf[RCV_BUF_SIZE];                    // receiver buffer
    int          res;
    CStreamer    Streamer(m_ClientSocket);                  // our streamer for UDP/TCP based RTP transport
    CRtspSession RtspSession(m_ClientSocket, &Streamer);     // our threads RTSP session and state
    int          StreamID = 0;                      // the ID of the 2 JPEG samples streams which we support
    HANDLE       WaitEvents[2];                     // the waitable kernel objects of our session

    HANDLE HTimer = ::CreateWaitableTimerA(NULL, false, NULL);

    WSAEVENT RtspReadEvent = ::WSACreateEvent();      // create READ wait event for our RTSP client socket
    ::WSAEventSelect(m_ClientSocket, RtspReadEvent, FD_READ);   // select socket read event
    WaitEvents[0] = RtspReadEvent;
    WaitEvents[1] = HTimer;

    // set frame rate timer
    const __int64 iT = 40;
    const __int64 DueTime = - iT * 10 * 1000;
    if (HTimer != 0)
    {
        ::SetWaitableTimer(HTimer, reinterpret_cast<const LARGE_INTEGER*>(&DueTime), iT, NULL, NULL, false);
    }
    
    bool StreamingStarted = false;
    bool Stop = false;
    RTSP_CMD_TYPES C;

    while ( !Stop && m_RtspMaster->getRtspRunning() )
    {
        switch (::WaitForMultipleObjects(2, WaitEvents, false, INFINITE))
        {
            case WAIT_OBJECT_0 + 0:
            {   // read client socket
                ::WSAResetEvent(WaitEvents[0]);

                std::memset(RecvBuf, 0x00, RCV_BUF_SIZE);
                res = ::recv(m_ClientSocket, RecvBuf, RCV_BUF_SIZE, 0);

                if (res <= 0)
                {
                    Stop = true;
                }
                else if (('O' == RecvBuf[0]) || ('D' == RecvBuf[0]) || ('S' == RecvBuf[0]) || ('P' == RecvBuf[0]) || ('T' == RecvBuf[0]))
                { // we filter away everything which seems not to be an RTSP command: O-ption, D-escribe, S-etup, P-lay, T-eardown
                    C = RtspSession.Handle_RtspRequest(RecvBuf, res);
                    if (RTSP_CMD_TYPES::RTSP_PLAY == C) 
                    {
                        StreamingStarted = true;
                    }
                    else if (RTSP_CMD_TYPES::RTSP_TEARDOWN == C)
                    {
                        Stop = true;
                    }
                }
                break;
            }
            case WAIT_OBJECT_0 + 1:
            {
                if (StreamingStarted)
                {
                    Streamer.StreamImage((CSampleModel*)m_RtspMaster->getSampleModel());
                }
                break;
            }
        }
    }
    m_RtspMaster->detachRtspClient(this);
}
