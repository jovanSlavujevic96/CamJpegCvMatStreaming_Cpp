#pragma once

#define MULTICAST_IPV4 "224.0.0.251"
#define MULTICAST_IPv6 "ff0e::"

#define PORT 4321
#define MAX_UDP_PAYLOAD_SIZE 65507

#define CAMERA_HEIGHT 480
#define CAMERA_WIDTH 640

#ifdef WIN

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <SDKDDKVer.h>
#include <Winsock2.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>  
#pragma comment(lib, "Ws2_32.lib")

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

typedef SOCKET int;
#define INVALID_SOCKET -1

#endif //WIN

#include <cstdio>
#include <cstdlib>
#include <tchar.h>

inline void _CloseSocket(SOCKET socket, bool cleanup=true)
{
#ifdef WIN
	closesocket(socket);
#else
	close(socket);
	if (cleanup)
	{
		WSACleanup();
	}
#endif //WIN
}

inline void _PrintSocketError(const char* errorMsg)
{
#ifdef WIN
	static char msgbuf[256];
	int err = WSAGetLastError();

	memset(msgbuf, 0, sizeof(msgbuf));
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
		NULL,                // lpsource
		err,                 // message id
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
		msgbuf,              // output buffer
		sizeof(msgbuf),     // size of msgbuf, bytes
		NULL);               // va_list of arguments

	printf("%s with error code %d\n", errorMsg, err);
	
	if (!*msgbuf)
		sprintf(msgbuf, "%d", err);  // provide error # if no string available
	printf("Error %s\n", msgbuf);
#else
	perror(errorMsg);
#endif //WIN
}
