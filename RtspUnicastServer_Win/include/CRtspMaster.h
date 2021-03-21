#pragma once

#pragma warning( disable : 4290 )

#include <thread>
#include <vector>
#include <memory>

#include "utils.h"
#include "CException.h"
#include "CSampleModel.h"
#include "CRtspClientHandler.h"

class CRtspMaster
{
public:
    CRtspMaster() throw(CException);
    ~CRtspMaster();

    bool getRtspRunning() const;
    void detachRtspClient(CRtspClientHandler* rtspClientToDetach);
    
    void startRtsp();
    void quitRtsp();
    void waitRtsp();

    void bindModel(CSampleModel* sampleModel);
    const CSampleModel* getSampleModel() const;
private:
    void MasterInit() throw(CException);
    void ClientAccepting();

    SOCKET      MasterSocket;                                 // our masterSocket(socket that listens for RTSP client connections)  
    SOCKET      ClientSocket;                                 // RTSP socket to handle an client
    sockaddr_in ServerAddr;                                   // server address parameters
    sockaddr_in ClientAddr;                                   // address parameters of a new RTSP client
    int         ClientAddrLen = sizeof(ClientAddr);
    bool        bRtspRunning;

    std::thread m_WorkingThread;                                /* Thread which handles receiving/accepting of new Rtsp clients */
    std::vector<std::unique_ptr<CRtspClientHandler>> RtspClients;
    CSampleModel* m_SampleModel;
};
