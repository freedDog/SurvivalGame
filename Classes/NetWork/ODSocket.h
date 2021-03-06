#pragma once

#ifndef _ODSOCKET_H_
#define _ODSOCKET_H_

#ifdef WIN32
#include <winsock.h>
typedef int				socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include "errno.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
typedef int				SOCKET;

//#pragma region define win32 const variable in linux
#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1
//#pragma endregion
#endif

class ODSocket {

public:
	ODSocket(SOCKET sock = INVALID_SOCKET);
	~ODSocket();

	// Connect socket
	bool Connect(const char* ip, unsigned short port);

	bool Check(void);
	bool hasError();         // 是否发生错误，注意，异步模式未完成非错误  

							 //#region server
							 // Bind socket
	bool Bind(unsigned short port);

	// Listen socket
	bool Listen(int backlog = 5);

	// Accept socket
	bool Accept(ODSocket& s, char* fromip = NULL);
	//#endregion

	// Send socket
	int Send(const char* buf, int len, int flags = 0);

	// Recv socket
	int Recv(char* buf, int len, int flags = 0);

	// Close socket
	int Close();

	// Get errno
	int GetError();

	// Domain parse
	static bool DnsParse(const char* domain, char* ip);

	ODSocket& operator = (SOCKET s);

	operator SOCKET ();

protected:
	SOCKET m_sock;

	// Create socket object for snd/recv data
	bool Create(int af, int type, int protocol = 0);

	//#pragma region just for win32
	// Init winsock DLL 
	static int Init();
	// Clean winsock DLL
	static int Clean();
	//#pragma endregion

};

#endif