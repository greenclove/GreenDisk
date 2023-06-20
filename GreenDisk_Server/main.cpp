#include <iostream>


#include "src/MysqlConnect.h"

INITIALIZE_EASYLOGGINGPP

int main() {
	el::Configurations conf("./config/easyloggingpp.conf");
	el::Loggers::reconfigureAllLoggers(conf);

	LOG(INFO) << "GreenDisk服务器启动中...";

	DBConnection dbCon;
	dbCon.host = "192.168.232.138";
	dbCon.user = "root";
	dbCon.passwd = "0000";
	dbCon.port = 3306;
	dbCon.db = "mysql";
	dbCon.unix_socket = NULL;
	dbCon.client_flag = 0;

	MysqlConnect mysqlConn(dbCon);
	mysqlConn.open();
	mysqlConn.close();

	return 0;
}
