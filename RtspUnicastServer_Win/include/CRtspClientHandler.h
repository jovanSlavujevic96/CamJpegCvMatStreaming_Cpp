#pragma once

#include <thread>

#include "utils.h"

class CRtspMaster;

class CRtspClientHandler
{
public:
	CRtspClientHandler(SOCKET ClientSocket, CRtspMaster* rtspMaster);
	~CRtspClientHandler();

	SOCKET getSocket() const;
private:
	void RtspClientSessionHandling();

	CRtspMaster* m_RtspMaster;
	SOCKET m_ClientSocket;
	std::thread m_WorkingThread;
};
