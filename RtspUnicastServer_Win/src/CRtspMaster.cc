#include <iostream>
#include <chrono>

#include <WS2tcpip.h>

#include "CRtspMaster.h"

CRtspMaster::CRtspMaster() throw(CException) :
	bRtspRunning{ true }
{
	MasterInit();
}

CRtspMaster::~CRtspMaster() = default;

bool CRtspMaster::getRtspRunning() const
{
	return bRtspRunning;
}

void CRtspMaster::detachRtspClient(CRtspClientHandler* rtspClientToDetach)
{
	unsigned int index = 0;
	for (std::unique_ptr<CRtspClientHandler>& rtspClient : RtspClients)
	{
		if (rtspClientToDetach == rtspClient.get())
		{
			rtspClient.release();
			RtspClients.erase(RtspClients.begin() + index);
			break;
		}
		index++;
	}
}

void CRtspMaster::startRtsp()
{
	m_WorkingThread = std::thread(&CRtspMaster::ClientAccepting, this);
}

void CRtspMaster::quitRtsp()
{
	shutdown(MasterSocket, SD_BOTH/*SHUT_RDWR*/);
	closesocket(MasterSocket);
	for (std::unique_ptr<CRtspClientHandler>& rtspClient : RtspClients)
	{
		shutdown(rtspClient->getSocket(), SD_BOTH/*SHUT_RDWR*/);
		closesocket(rtspClient->getSocket());
	}
}

void CRtspMaster::waitRtsp()
{
	m_WorkingThread.join();
}

void CRtspMaster::bindModel(CSampleModel* sampleModel)
{
	m_SampleModel = sampleModel;
}

const CSampleModel* CRtspMaster::getSampleModel() const
{
	return m_SampleModel;
}

void CRtspMaster::MasterInit() throw(CException)
{
	WSADATA     WsaData;
	int ret = WSAStartup(0x101, &WsaData);
	if (ret != 0)
	{
		throw CException("CRtspMaster :: WSAStartup failed");
	}

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = INADDR_ANY;
	ServerAddr.sin_port = htons(8554);                 // listen on RTSP port 8554
	MasterSocket = socket(AF_INET, SOCK_STREAM, 0);

	// bind our master socket to the RTSP port and listen for a client connection
	if (bind(MasterSocket, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) != 0)
	{
		throw CException("CRtspMaster :: bind MasterSocket failed");
	}
	else if (listen(MasterSocket, 5) != 0)
	{
		throw CException("CRtspMaster :: listen MasterSocket failed");
	}
}

void CRtspMaster::ClientAccepting()
{
	const size_t IPv4AddrStrSize = 100u;
	char IPv4ClientAddrStr[IPv4AddrStrSize] = { 0 };
	while (bRtspRunning)
	{   // loop forever to accept client connections
		ClientSocket = accept(MasterSocket, (struct sockaddr*)&ClientAddr, &ClientAddrLen);
		if (INVALID_SOCKET == ClientSocket)
		{
			break;
		}
		RtspClients.push_back(std::unique_ptr<CRtspClientHandler>(new CRtspClientHandler(ClientSocket, this)));
		if (inet_ntop(AF_INET, &ClientAddr.sin_addr, IPv4ClientAddrStr, IPv4AddrStrSize))
		{
			std::cout << "Client connected. Client address: " << IPv4ClientAddrStr << std::endl;
		}
		else
		{
			break;
		}
	}
}
