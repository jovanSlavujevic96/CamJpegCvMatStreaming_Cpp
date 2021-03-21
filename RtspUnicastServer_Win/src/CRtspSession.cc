// MediaLAN 02/2013
// CRtspSession
// - parsing of RTSP requests and generation of RTSP responses

#include <iostream>
#include <stdio.h>
#include <time.h>

#include "CStreamer.h"
#include "CRtspSession.h"

CRtspSession::CRtspSession(SOCKET aRtspClient, CStreamer * aStreamer) :
    m_RtspClient{ aRtspClient },
    m_Streamer{ aStreamer },
    CmdName{ 0 }
{
    Init();

    m_RtspSessionID  = rand() << 16;         // create a session ID
    m_RtspSessionID |= rand();
    m_RtspSessionID |= 0x80000000;         
    m_StreamID       = -1;
    m_ClientRTPPort  =  0;
    m_ClientRTCPPort =  0;
    m_TcpTransport   =  false;
};

CRtspSession::~CRtspSession() = default;

void CRtspSession::Init()
{
    m_RtspCmdType = RTSP_CMD_TYPES::RTSP_UNKNOWN;
    memset(m_URLPreSuffix, 0x00, RTSP_PARAM_STRING_MAX);
    memset(m_URLSuffix,    0x00, RTSP_PARAM_STRING_MAX);
    memset(m_CSeq,         0x00, RTSP_PARAM_STRING_MAX);
    memset(m_URLHostPort,  0x00, RTSP_BUFFER_SIZE);
    memset(CmdName,        0x00, RTSP_PARAM_STRING_MAX);
    m_ContentLength  =  0;
}

bool CRtspSession::ParseRtspRequest(char const* aRequest, unsigned aRequestSize)
{
    Init();

    // check whether the request contains information about the RTP/RTCP UDP client ports (SETUP command)
    char* ClientPortPtr;
    char* TmpPtr;
    char  CP[1024];
    char* pCP;

    if (aRequest && aRequestSize > 0)
    {
        ClientPortPtr = strstr((char*)aRequest, "client_port");
    }
    else
    {
        return false;
    }
    if (ClientPortPtr != nullptr)
    {
        TmpPtr = strstr(ClientPortPtr, "\r\n");
        if (TmpPtr != nullptr)
        {
            TmpPtr[0] = 0x00;
            strcpy_s(CP, ClientPortPtr);
            pCP = strstr(CP, "=");
            if (pCP != nullptr)
            {
                pCP++;
                strcpy_s(CP, pCP);
                pCP = strstr(CP, "-");
                if (pCP != nullptr)
                {
                    pCP[0] = 0x00;
                    m_ClientRTPPort = atoi(CP);
                    m_ClientRTCPPort = m_ClientRTPPort + 1;
                }
            }
        }
    }

    // Read everything up to the first space as the command name
    bool parseSucceeded = false;
    unsigned i;
    for (i = 0; i < sizeof(CmdName) - 1 && i < aRequestSize; ++i)
    {
        const char& c = aRequest[i];
        if (c == ' ' || c == '\t')
        {
            parseSucceeded = true;
            break;
        }
        CmdName[i] = c;
    }
    CmdName[i] = '\0';
    if (!parseSucceeded)
    {
        return false;
    }

    // find out the command type
    if (strstr(CmdName, "OPTIONS"))
    {
        m_RtspCmdType = RTSP_CMD_TYPES::RTSP_OPTIONS;
    }
    else if (strstr(CmdName, "DESCRIBE"))
    {
        m_RtspCmdType = RTSP_CMD_TYPES::RTSP_DESCRIBE;
    }
    else if (strstr(CmdName, "SETUP"))
    {
        m_RtspCmdType = RTSP_CMD_TYPES::RTSP_SETUP;
    }
    else if (strstr(CmdName, "PLAY"))
    {
        m_RtspCmdType = RTSP_CMD_TYPES::RTSP_PLAY;
    }
    else if (strstr(CmdName, "TEARDOWN"))
    {
        m_RtspCmdType = RTSP_CMD_TYPES::RTSP_TEARDOWN;
    }

    // check whether the request contains transport information (UDP or TCP)
    if (RTSP_CMD_TYPES::RTSP_SETUP == m_RtspCmdType)
    {
        TmpPtr = strstr((char*)aRequest,"RTP/AVP/TCP");
        if (TmpPtr != nullptr)
        {
            m_TcpTransport = true;
        }
        else
        {
            m_TcpTransport = false;
        }
    };

    // Skip over the prefix of any "rtsp://" or "rtsp:/" URL that follows:
    unsigned j = i+1;
    while (j < aRequestSize && (aRequest[j] == ' ' || aRequest[j] == '\t')) ++j; // skip over any additional white space
    for (; (int)j < (int)(aRequestSize -8); ++j)
    {
        if(strstr(&aRequest[j],"rtsp:/") || strstr(&aRequest[j], "RTSP:/"))
        {
            j += 6;
            if ('/' == aRequest[j])
            {   // This is a "rtsp://" URL; skip over the host:port part that follows:
                ++j;
                unsigned uidx = 0;
                while (j < aRequestSize && aRequest[j] != '/' && aRequest[j] != ' ')
                {   // extract the host:port part of the URL here
                    m_URLHostPort[uidx] = aRequest[j];
                    uidx++;
                    ++j;
                };
            } 
            else --j;
            i = j;
            break;
        }
    }

    // Look for the URL suffix (before the following "RTSP/"):
    parseSucceeded = false;
    for (unsigned k = i+1; (int)k < (int)(aRequestSize -5); ++k)
    {
        if ('R' == aRequest[k]     && 'T' == aRequest[k + 1] &&
            'S' == aRequest[k + 2] && 'P' == aRequest[k + 3] &&
            '/' == aRequest[k+4])
        {
            while (--k >= i && aRequest[k] == ' ') {}
            unsigned k1 = k;
            while (k1 > i && aRequest[k1] != '/') --k1;
            if ((k - k1 + 1) > sizeof(m_URLSuffix))
            {
                return false;
            }
            unsigned n = 0, k2 = k1+1;

            while (k2 <= k) m_URLSuffix[n++] = aRequest[k2++];
            m_URLSuffix[n] = '\0';

            if (k1 - i > sizeof(m_URLPreSuffix)) return false;
            n = 0; k2 = i + 1;
            while (k2 <= k1 - 1) m_URLPreSuffix[n++] = aRequest[k2++];
            m_URLPreSuffix[n] = '\0';
            i = k + 7; 
            parseSucceeded = true;
            break;
        }
    }
    if (!parseSucceeded) return false;

    // Look for "CSeq:", skip whitespace, then read everything up to the next \r or \n as 'CSeq':
    parseSucceeded = false;
    for (j = i; (int)j < (int)(aRequestSize -5); ++j)
    {
        //if(strstr(&aRequest[j],"CSeq:"))
        if (aRequest[j]   == 'C' && aRequest[j+1] == 'S' && 
            aRequest[j+2] == 'e' && aRequest[j+3] == 'q' && 
            aRequest[j+4] == ':') 
        {
            j += 5;
            while (j < aRequestSize && (aRequest[j] ==  ' ' || aRequest[j] == '\t')) ++j;
            unsigned n;
            for (n = 0; n < sizeof(m_CSeq)-1 && j < aRequestSize; ++n,++j)
            {
                char c = aRequest[j];
                if (c == '\r' || c == '\n') 
                {
                    parseSucceeded = true;
                    break;
                }
                m_CSeq[n] = c;
            }
            m_CSeq[n] = '\0';
            break;
        }
    }
    if (!parseSucceeded)
    {
        return false;
    }

    // Also: Look for "Content-Length:" (optional)
    for (j = i; (int)j < (int)(aRequestSize -15); ++j)
    {
        if(strstr(&aRequest[j],"Content-Length:") || strstr(&aRequest[j], "Content-length:"))
        {
            j += 15;
            while (j < aRequestSize && (aRequest[j] ==  ' ' || aRequest[j] == '\t')) ++j;
            unsigned num;
            if (sscanf_s(&aRequest[j], "%u", &num) == 1) m_ContentLength = num;
        }
    }
    return true;
}

RTSP_CMD_TYPES CRtspSession::Handle_RtspRequest(char const * aRequest, unsigned aRequestSize)
{
    if (ParseRtspRequest(aRequest,aRequestSize))
    {
        switch (m_RtspCmdType)
        {
            case RTSP_CMD_TYPES::RTSP_OPTIONS:  
            { 
                Handle_RtspOPTION();
                break; 
            }
            case RTSP_CMD_TYPES::RTSP_DESCRIBE: 
            { 
                Handle_RtspDESCRIBE(); 
                break; 
            }
            case RTSP_CMD_TYPES::RTSP_SETUP:
            { 
                Handle_RtspSETUP();
                break; 
            }
            case RTSP_CMD_TYPES::RTSP_PLAY:
            { 
                Handle_RtspPLAY();
                break; 
            }
            default: {}
        }
    }
    return m_RtspCmdType;
}

void CRtspSession::Handle_RtspOPTION()
{
    /* clear stringstream */
    m_StreamingMessage.str("");

    /* generate message */
    m_StreamingMessage << "RTSP/1.0 200 OK\r\nCSeq: " << m_CSeq << "\r\n" \
        << "Public: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE\r\n\r\n";

    /* get strlen of stringstream */
    m_StreamingMessage.seekg(0, std::ios::end);
    int size = (int)m_StreamingMessage.tellg();

    send(m_RtspClient, m_StreamingMessage.str().c_str(), size, 0);
}

void CRtspSession::Handle_RtspDESCRIBE()
{
    /* clear stringstream */
    m_StreamingMessage.str("");

    char   SDPBuf[100];

    // check whether we know a stream with the URL which is requested
    m_StreamID = -1;        // invalid URL
    if ((strcmp(m_URLPreSuffix, "mjpeg") == 0) && (strcmp(m_URLSuffix, "1") == 0))
    {
        m_StreamID = 0;
    }
    else if ((strcmp(m_URLPreSuffix, "mjpeg") == 0) && (strcmp(m_URLSuffix, "2") == 0))
    {
        m_StreamID = 1;
    }
    
    if (m_StreamID == -1)
    {   // Stream not available
        
        /* generate message */
        m_StreamingMessage << "RTSP/1.0 404 Stream Not Found\r\nCSeq: "<< m_CSeq << "\r\n" << DateHeader() << "\r\n";

        /* get strlen of stringstream */
        m_StreamingMessage.seekg(0, std::ios::end);
        int size = (int)m_StreamingMessage.tellg();

        send(m_RtspClient, m_StreamingMessage.str().c_str(), size, 0);
        return;
    };

    // simulate DESCRIBE server response
    char * ColonPtr;
    ColonPtr = strstr(m_URLHostPort,":");
    if (ColonPtr != nullptr)
    {
        ColonPtr[0] = 0x00;
    }

    _snprintf_s(SDPBuf, sizeof(SDPBuf),
        "v=0\r\n"
        "o=- %d 1 IN IP4 %s\r\n"
        "s=\r\n"
        "t=0 0\r\n"                                            // start / stop - 0 -> unbounded and permanent session
        "m=video 0 RTP/AVP 26\r\n"                             // currently we just handle UDP sessions
        "c=IN IP4 0.0.0.0\r\n",
        rand(),
        m_URLHostPort);

    m_StreamingMessage << "RTSP/1.0 200 OK\r\nCSeq: " << m_CSeq << "\r\n" \
        << DateHeader() << "\r\n" \
        << "Content-Base: " << "rtsp://" << m_URLHostPort << "/mjpeg/" << m_StreamID + 1 << "/\r\n" \
        << "Content-Type: application/sdp\r\n" \
        << "Content-Length: " << strlen(SDPBuf) << "\r\n\r\n" << SDPBuf;

    /* get strlen of stringstream */
    m_StreamingMessage.seekg(0, std::ios::end);
    int size = (int)m_StreamingMessage.tellg();

    send(m_RtspClient, m_StreamingMessage.str().c_str(), size, 0);
}

void CRtspSession::Handle_RtspSETUP()
{
    /* clear stringstream */
    m_StreamingMessage.str("");

    // init RTP streamer transport type (UDP or TCP) and ports for UDP transport
    m_Streamer->InitTransport(m_ClientRTPPort,m_ClientRTCPPort,m_TcpTransport);

    m_StreamingMessage << "RTSP/1.0 200 OK\r\nCSeq: " << m_CSeq << "\r\n" \
        << DateHeader() << "\r\n" << "Transport: "; 
    
    // simulate SETUP server response
    if (m_TcpTransport)
    {
        m_StreamingMessage << "RTP/AVP/TCP;unicast;interleaved=0-1\r\n";
    }
    else
    {
        m_StreamingMessage << "RTP/AVP;unicast;destination=127.0.0.1;source=127.0.0.1;client_port=" << m_ClientRTPPort << '-' \
            << m_ClientRTCPPort << ";server_port=" << m_Streamer->GetRtpServerPort() << '-' << m_Streamer->GetRtcpServerPort() << "\r\n";
    }
    m_StreamingMessage << "Session: " << m_RtspSessionID << "\r\n\r\n";

    /* get strlen of stringstream */
    m_StreamingMessage.seekg(0, std::ios::end);
    int size = (int)m_StreamingMessage.tellg();

    send(m_RtspClient, m_StreamingMessage.str().c_str(), size, 0);
}

void CRtspSession::Handle_RtspPLAY()
{
    /* clear stringstream */
    m_StreamingMessage.str("");

    m_StreamingMessage << "RTSP/1.0 200 OK\r\nCSeq: " << m_CSeq << "\r\n" \
        << DateHeader() << "\r\nRange: npt=0.000-\r\nSession: " << m_RtspSessionID << "\r\n" \
        << "RTP-Info: url=rtsp://127.0.0.1:8554/mjpeg/1/track1\r\n\r\n";

    /* get strlen of stringstream */
    m_StreamingMessage.seekg(0, std::ios::end);
    int size = (int)m_StreamingMessage.tellg();

    send(m_RtspClient, m_StreamingMessage.str().c_str(), size, 0);
}

char const * CRtspSession::DateHeader() 
{
    static char buf[200];    
    time_t tt = time(NULL);
    tm timeOutput;
    int error = gmtime_s(&timeOutput, &tt);
    if (error)
    {
        return NULL;
    }
    strftime(buf, sizeof buf, "Date: %a, %b %d %Y %H:%M:%S GMT", &timeOutput);
    return buf;
}

int CRtspSession::GetStreamID() const
{
    return m_StreamID;
}
