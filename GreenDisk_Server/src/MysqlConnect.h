//
// Created by AdminWJH on 2023/05/24.
//

#ifndef GREENDISK_SERVER_MYSQLCONNECT_H
#define GREENDISK_SERVER_MYSQLCONNECT_H

#include "globalSQL.h"


class MysqlConnect {
public:
	MysqlConnect(DBConnection dbCon);
	~MysqlConnect();

	ReturnMessage       open();
	void                close();

	/// @brief 执行SQL
	ReturnMessage       execute(string sql);

	LstData             oneFieldQuery(TableInfo tableInfo, string fieldName);
	LstData             oneFieldQuery(string sql);
	TableData           query(TableInfo tableInfo);
	TableData           query(string sql);

	/// @brief MYSQL_BIND
	ReturnMessage       bindInsert(TableInfo tableInfo, TableData tableData);

private:
	ReturnMessage doLog(string classFunc = "", string mysqlFunc = "", string sqlData = "");

private:
	MYSQL mysql;

	bool isOpen = false;

	DBConnection m_dbCon;
};


#endif //GREENDISK_SERVER_MYSQLCONNECT_H
