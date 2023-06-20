//
// Created by AdminWJH on 2023/05/24.
//

#ifndef GREENDISK_SERVER_DEFINE_H
#define GREENDISK_SERVER_DEFINE_H


#include "Format.h"
#include "winsock.h"
#include <direct.h>
#include <string>
#include <iostream>
#include <list>
#include "easylogging++.h"

using namespace std;

struct ReturnMessage
{
	bool    isSuccess = true;
	bool    boolValue1;
	bool    boolValue2;
	bool    boolValue3;
	string  message;
	string  stringValue1;
	string  stringValue2;
	string  stringValue3;
	int     returnCode = 0;
	int     intValue1;
	int     intValue2;
	int     intValue3;
};

#endif //GREENDISK_SERVER_DEFINE_H
