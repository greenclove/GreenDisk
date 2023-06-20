//
// Created by AdminWJH on 2023/05/25.
//

#include "Utils.h"

string Utils::getAppPath() {
	char path[256];

	_getcwd(path, 256);

	return path;
}
