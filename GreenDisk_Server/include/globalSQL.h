#ifndef GLOBALSQL_H
#define GLOBALSQL_H

#include "define.h"
#include "mysql.h"
#include "mysql_time.h"
#include "binary_log_types.h"

/// @brief mysql数据库连接信息
struct DBConnection
{
	DBConnection(){}
	DBConnection(string _host, string _user, string _passwd, string _db, int _port, string _unix_socket = NULL, long _client_flag = 0)
	{
		host        = _host.c_str();
		user        = _user.c_str();
		passwd      = _passwd.c_str();
		db          = _db.c_str();
		port        = _port;
		unix_socket = _unix_socket.c_str();
		client_flag = _client_flag;
	}

	const char *    host;
	const char *    user;
	const char *    passwd;
	const char *    db;
	unsigned int    port;
	const char *    unix_socket = nullptr;
	unsigned long   client_flag = 0;

};

enum TABLEKEY
{
	TK_NO,
	TK_PRIMARY,
	TK_FOREIGN
};

enum MYSQLDATATYPE
{
	MDT_TINYINT,
	MDT_SMALLINT,
	MDT_MEDIUMINT,
	MDT_INTEGER,
	MDT_BIGINT,
	MDT_FLOAT,
	MDT_DOUBLE,
	MDT_DECIMAL,
	MDT_DATE,
	MDT_TIME,
	MDT_YEAR,
	MDT_DATETIME,
	MDT_TIMESTAMP,
	MDT_CHAR,
	MDT_VARCHAR,
	MDT_TINYBLOB,
	MDT_TINYTEXT,
	MDT_BLOB,
	MDT_BINARY,
	MDT_VARBINARY,
	MDT_TEXT,
	MDT_MEDIUMBLOB,
	MDT_MEDIUMTEXT,
	MDT_LONGBLOB,
	MDT_LONGTEXT,
	MDT_BIT
};

/// @brief 表字段信息
struct FieldInfo
{
	string          fieldName;
	MYSQLDATATYPE   fieldType;
	TABLEKEY        tablekey;
	unsigned long   precision;
	unsigned long   scale;
	bool            nullAble;
	char *          defauleValue;
};

/// @brief 表信息
struct TableInfo
{
	string              schema;
	string              name;
	vector<FieldInfo>   fieldsInfo;
};

/// @brief 字段值
struct FieldValue
{
	/// @brief 字段值长度
	unsigned long   length;
	/// @brief 字段值
	char * value;
};

/// @brief mysql表数据列表
typedef struct LstData
{
	ReturnMessage       rm;
	vector<FieldValue>  data;
}LstData;

/// @brief 表数据，二维列表，行列式
typedef struct TableData
{
	ReturnMessage       rm;
	vector<LstData>     data;
}TableData;



#endif // GLOBALSQL_H
