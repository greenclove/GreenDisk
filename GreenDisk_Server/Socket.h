//
// Created by AdminWJH on 2023/06/04.
//

#ifndef GREENDISK_SERVER_SOCKET_H
#define GREENDISK_SERVER_SOCKET_H

#include <WinSock2.h>
#include "define.h"


class Socket {
public:
	Socket();
	~Socket();

	ReturnMessage   create(int af, int type, int protocol = 0);
	ReturnMessage   bind(string& ip, unsigned short port);
	ReturnMessage


};


#endif //GREENDISK_SERVER_SOCKET_H
