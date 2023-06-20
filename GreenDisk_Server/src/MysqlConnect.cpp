//
// Created by AdminWJH on 2023/05/24.
//

#include "MysqlConnect.h"

MysqlConnect::MysqlConnect(DBConnection dbCon) {
	m_dbCon = dbCon;
}

ReturnMessage MysqlConnect::open() {
	ReturnMessage RM;


	if(!mysql_init(&mysql))
	{
		return doLog("open", "mysql_init");
	}


	if(!mysql_real_connect(&mysql,m_dbCon.host,m_dbCon.user,m_dbCon.passwd,m_dbCon.db,m_dbCon.port,m_dbCon.unix_socket,m_dbCon.client_flag))
	{
		return doLog("open", "mysql_real_connect");
	}

	LOG(INFO) << "-----Connect database success-----";

	isOpen = true;
	return RM;
}

void MysqlConnect::close() {
	if(isOpen)
		mysql_close(&mysql);
}

MysqlConnect::~MysqlConnect() {
	close();
}

ReturnMessage MysqlConnect::execute(string sql) {
	ReturnMessage RM;

	if(!mysql_real_query(&mysql, sql.data(), sql.length()))
	{
		return doLog("execute", "mysql_real_query", sql);
	}

	return RM;
}

LstData MysqlConnect::oneFieldQuery(TableInfo tableInfo, string fieldName) {
	LstData lstData;
	string sql = util::Format("select {0} from {`1`}.{`2`}", fieldName, tableInfo.schema, tableInfo.name);

	return oneFieldQuery(sql);
}

LstData MysqlConnect::oneFieldQuery(string sql) {
	LstData lstData;
	mysql_real_query(&mysql, sql.data(), sql.length());
	if(mysql_errno(&mysql))
	{
		ReturnMessage RM = doLog("oneFieldQuery", "mysql_real_query", sql);
		lstData.rm = RM;
		return lstData;
	}

	MYSQL_RES * res = mysql_store_result(&mysql);
	if(mysql_errno(&mysql))
	{
		ReturnMessage RM = doLog("oneFieldQuery", "mysql_store_result");
		lstData.rm = RM;
		return lstData;
	}

	int fieldCount = mysql_field_count(&mysql);
	if(mysql_errno(&mysql))
	{
		ReturnMessage RM = doLog("oneFieldQuery", "mysql_field_count");
		lstData.rm = RM;
		return lstData;
	}

	if(fieldCount == 0)
	{
		ReturnMessage RM;
		RM.returnCode = -1;
		RM.message = "Execute failure，Probably not an SQL!";
		lstData.rm  = RM;
	}
	else if(fieldCount == 1)
	{
		while (MYSQL_ROW row = mysql_fetch_row(res))
		{
			unsigned long * lengths;
			lengths = mysql_fetch_lengths(res);
			FieldValue fieldValue;
			fieldValue.length = lengths[0];
			fieldValue.value = (char *)row[0];
			lstData.data.push_back(fieldValue);
		}
	}
	else
	{
		ReturnMessage RM;
		RM.returnCode = -2;
		RM.message = "Execute Failure，SQL Query result have multi field!";
		lstData.rm  = RM;
	}

	mysql_free_result(res);
	return lstData;
}

ReturnMessage MysqlConnect::doLog(string classFunc, string mysqlFunc, string sqlData) {
	ReturnMessage RM;
	RM.isSuccess = false;
	RM.returnCode = mysql_errno(&mysql);
	RM.message = mysql_error(&mysql);
	LOG(ERROR) << util::Format("MysqlConnect::{0}-{1} Error---ErrorCode:{2};ErrorText:{3}", classFunc, mysqlFunc, RM.returnCode, RM.message).data();
	if(sqlData.length() > 0)
		LOG(ERROR) << sqlData.data();
	return RM;
}

TableData MysqlConnect::query(TableInfo tableInfo) {
	string sql = util::Format("select * from {`0`}.{`1`}", tableInfo.schema, tableInfo.name);

	return query(sql);
}

TableData MysqlConnect::query(string sql) {
	TableData tableData;

	mysql_real_query(&mysql, sql.data(), sql.length());
	if(mysql_errno(&mysql))
	{
		ReturnMessage RM = doLog("oneFieldQuery", "mysql_real_query", sql);
		tableData.rm = RM;
		return tableData;
	}

	MYSQL_RES * res = mysql_store_result(&mysql);
	if(mysql_errno(&mysql))
	{
		ReturnMessage RM = doLog("oneFieldQuery", "mysql_store_result");
		tableData.rm = RM;
		return tableData;
	}

	int fieldCount = mysql_field_count(&mysql);
	if(mysql_errno(&mysql))
	{
		ReturnMessage RM = doLog("oneFieldQuery", "mysql_field_count");
		tableData.rm = RM;
		return tableData;
	}

	if(fieldCount == 0)
	{
		ReturnMessage RM;
		RM.returnCode = -1;
		RM.message = "Execute failure，Probably not an SQL!";
		tableData.rm  = RM;
	}
	else
	{
		while (MYSQL_ROW row = mysql_fetch_row(res))
		{
			unsigned long * lengths;
			lengths = mysql_fetch_lengths(res);
			LstData lstData;
			for(int fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++) {
				FieldValue fieldValue;
				fieldValue.length = lengths[fieldIndex];
				fieldValue.value = (char *)row[fieldIndex];
				lstData.data.push_back(fieldValue);
			}
			tableData.data.push_back(lstData);
		}
	}

	mysql_free_result(res);

	return tableData;
}

ReturnMessage MysqlConnect::bindInsert(TableInfo tableInfo, TableData tableData) {
	ReturnMessage RM;

	int rowCount = tableData.data.size();
	int columnCount = tableInfo.fieldsInfo.size();

	string sql, columnNameList, valueList;
	for(int columnIndex = 0; columnIndex < columnCount; columnIndex++)
	{
		if(columnIndex < columnCount - 1)
		{
			columnNameList.append(tableInfo.fieldsInfo.at(columnIndex).fieldName).append(", ");
			valueList.append("?, ");
		}
		else
		{
			columnNameList.append(tableInfo.fieldsInfo.at(columnIndex).fieldName);
			valueList.append("?");
		}
	}
	sql = util::Format("insert into {`0`}.{`1`} ({2}) values (3)", tableInfo.schema, tableInfo.name, columnNameList, valueList);

	MYSQL_STMT * stmt = mysql_stmt_init(&mysql);
	if(mysql_errno(&mysql))
	{
		return doLog("bindInsert", "mysql_stmt_init");
	}

	mysql_stmt_prepare(stmt, sql.data(), sql.length());
	if(mysql_stmt_errno(stmt))
	{

	}

	MYSQL_BIND bind[columnCount];
	memset(bind, 0, sizeof(bind));
	for(int columnIndex = 0; columnIndex < columnCount; columnIndex++)
	{
		bind[columnIndex].length = 0;
		bind[columnIndex].buffer = 0;
		bind[columnIndex].is_null = 0;
	}

	for(int columnIndex = 0; columnIndex < columnCount; columnIndex++)
	{
		MYSQLDATATYPE mdt_datatype = tableInfo.fieldsInfo.at(columnIndex).fieldType;

		bind[columnIndex].length = new unsigned long[rowCount];
		bind[columnIndex].is_null= new my_bool[rowCount];

		if(mdt_datatype == MDT_TINYINT)
		{
			bind[columnIndex].buffer_type = MYSQL_TYPE_TINY;
			bind[columnIndex].buffer = new char[rowCount];
		}
		else if(mdt_datatype == MDT_SMALLINT || mdt_datatype == MDT_YEAR)
		{
			bind[columnIndex].buffer_type = MYSQL_TYPE_SHORT;
			bind[columnIndex].buffer = new short int[rowCount];
		}
		else if(mdt_datatype == MDT_MEDIUMINT)
		{
			bind[columnIndex].buffer_type = MYSQL_TYPE_INT24;
			bind[columnIndex].buffer = new int[rowCount];
		}
		else if(mdt_datatype == MDT_INTEGER)
		{
			bind[columnIndex].buffer_type = MYSQL_TYPE_LONG;
			bind[columnIndex].buffer = new int[rowCount];
		}
		else if(mdt_datatype == MDT_BIGINT)
		{
			bind[columnIndex].buffer_type = MYSQL_TYPE_LONGLONG;
			bind[columnIndex].buffer = new long long int[rowCount];
		}
		else if(mdt_datatype == MDT_FLOAT)
		{
			bind[columnIndex].buffer_type = MYSQL_TYPE_FLOAT;
			bind[columnIndex].buffer = new float[rowCount];
		}
		else if(mdt_datatype == MDT_DOUBLE)
		{
			bind[columnIndex].buffer_type = MYSQL_TYPE_DOUBLE;
			bind[columnIndex].buffer = new double [rowCount];
		}
		else if(mdt_datatype == MDT_DECIMAL)
		{
			unsigned long fieldPrecision = tableInfo.fieldsInfo.at(columnIndex).precision;
			bind[columnIndex].buffer_type = MYSQL_TYPE_NEWDECIMAL;
			bind[columnIndex].buffer = new char [rowCount * fieldPrecision];
		}
		else if(mdt_datatype == MDT_TIME)
		{
			bind[columnIndex].buffer_type = MYSQL_TYPE_TIME;
			bind[columnIndex].buffer = new MYSQL_TIME[rowCount];
		}
		else if(mdt_datatype == MDT_DATE)
		{
			bind[columnIndex].buffer_type = MYSQL_TYPE_DATE;
			bind[columnIndex].buffer = new MYSQL_TIME[rowCount];
		}
		else if(mdt_datatype == MDT_DATETIME)
		{
			bind[columnIndex].buffer_type = MYSQL_TYPE_DATETIME;
			bind[columnIndex].buffer = new MYSQL_TIME[rowCount];
		}
		else if(mdt_datatype == MDT_TIMESTAMP)
		{
			bind[columnIndex].buffer_type = MYSQL_TYPE_TIMESTAMP;
			bind[columnIndex].buffer = new MYSQL_TIME[rowCount];
		}
		else if(mdt_datatype == MDT_CHAR || mdt_datatype == MDT_BINARY)
		{
			unsigned long fieldPrecision = tableInfo.fieldsInfo.at(columnIndex).precision;
			bind[columnIndex].buffer_type = MYSQL_TYPE_STRING;
			bind[columnIndex].buffer = new char[rowCount * fieldPrecision];
		}
		else if(mdt_datatype == MDT_VARCHAR || mdt_datatype == MDT_VARBINARY)
		{
			unsigned long fieldPrecision = tableInfo.fieldsInfo.at(columnIndex).precision;
			bind[columnIndex].buffer_type = MYSQL_TYPE_VAR_STRING;
			bind[columnIndex].buffer = new char[rowCount * fieldPrecision];
		}
		else if(mdt_datatype == MDT_TINYBLOB || mdt_datatype == MDT_TINYTEXT)
		{
			unsigned long fieldPrecision = tableInfo.fieldsInfo.at(columnIndex).precision;
			bind[columnIndex].buffer_type = MYSQL_TYPE_TINY_BLOB;
			bind[columnIndex].buffer = new char[rowCount * fieldPrecision];
		}
		else if(mdt_datatype == MDT_BLOB || mdt_datatype == MDT_TEXT)
		{
			unsigned long fieldPrecision = tableInfo.fieldsInfo.at(columnIndex).precision;
			bind[columnIndex].buffer_type = MYSQL_TYPE_BLOB;
			bind[columnIndex].buffer = new char[rowCount * fieldPrecision];
		}
		else if(mdt_datatype == MDT_MEDIUMBLOB || mdt_datatype == MDT_MEDIUMTEXT)
		{
			unsigned long fieldPrecision = tableInfo.fieldsInfo.at(columnIndex).precision;
			bind[columnIndex].buffer_type = MYSQL_TYPE_MEDIUM_BLOB;
			bind[columnIndex].buffer = new char[rowCount * fieldPrecision];
		}
		else if(mdt_datatype == MDT_LONGBLOB || mdt_datatype == MDT_LONGTEXT)
		{
			unsigned long fieldPrecision = tableInfo.fieldsInfo.at(columnIndex).precision;
			bind[columnIndex].buffer_type = MYSQL_TYPE_LONG_BLOB;
			bind[columnIndex].buffer = new char[rowCount * fieldPrecision];
		}
		else if(mdt_datatype == MDT_BIT)
		{
			unsigned long fieldPrecision = tableInfo.fieldsInfo.at(columnIndex).precision;
			bind[columnIndex].buffer_type = MYSQL_TYPE_BIT;
			bind[columnIndex].buffer = new char[rowCount * fieldPrecision];
		}
	}

	//开始向bind中绑定数据
	for(int rowIndex = 0; rowIndex < rowCount; rowIndex++)
	{
		for(int columnIndex = 0; columnIndex < columnCount; columnIndex++)
		{
			FieldInfo currField = tableInfo.fieldsInfo.at(columnIndex);
			MYSQLDATATYPE mdt_datatype = currField.fieldType;

			// 获取当前行，当前列字段的值
			FieldValue currFieldValue = tableData.data.at(rowIndex).data.at(columnIndex);

			if(mdt_datatype == MDT_TINYINT)
			{
				char *buffer = (char *)bind[columnIndex].buffer + sizeof(char) * rowIndex;
				if(currFieldValue.length > 0) {
					/*buffer = buffer + sizeof(char) * rowIndex;
					const char *value = (const char *) currFieldValue.value;
					memcpy(buffer, value, currFieldValue.length);*/
					buffer[rowIndex] = *currFieldValue.value;
					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{
						buffer[rowIndex] = *currField.defauleValue;
						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_SMALLINT || mdt_datatype == MDT_YEAR)
			{
				short * buffer = (short*)bind[columnIndex].buffer + sizeof(short) * rowIndex;
				if(currFieldValue.length > 0)
				{
					char * value = currFieldValue.value;
					memcpy(buffer, value, currFieldValue.length);
					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{
						char * value = (char*)(currField.defauleValue);
						memcpy(buffer, value, strlen(currField.defauleValue));
						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_MEDIUMINT)
			{
				int * buffer = (int*)bind[columnIndex].buffer + sizeof(int) * rowIndex;
				if(currFieldValue.length > 0)
				{
					char * value = currFieldValue.value;
					memcpy(buffer, value, currFieldValue.length);
					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{
						char * value = currField.defauleValue;
						memcpy(buffer, value, strlen(currField.defauleValue));
						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_INTEGER)
			{
				int * buffer = (int*)bind[columnIndex].buffer + sizeof(int) * rowIndex;
				if(currFieldValue.length > 0)
				{
					char * value = currFieldValue.value;
					memcpy(buffer, value, currFieldValue.length);
					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{
						char * value = currField.defauleValue;
						memcpy(buffer, value, strlen(currField.defauleValue));
						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_BIGINT)
			{
				long long * buffer = (long long*)bind[columnIndex].buffer  + sizeof(long long) * rowIndex;
				if(currFieldValue.length > 0)
				{
					char * value = currFieldValue.value;
					memcpy(buffer, value, currFieldValue.length);
					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{
						char * value = currField.defauleValue;
						memcpy(buffer, value, strlen(currField.defauleValue));
						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_FLOAT)
			{
				float * buffer = (float*)bind[columnIndex].buffer + sizeof(float) * rowIndex;
				if(currFieldValue.length > 0)
				{
					char * value = currFieldValue.value;
					memcpy(buffer, value, currFieldValue.length);
					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{
						char * value = currField.defauleValue;
						memcpy(buffer, value, strlen(currField.defauleValue));
						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_DOUBLE)
			{
				double * buffer = (double*)bind[columnIndex].buffer + sizeof(double) * rowIndex;
				if(currFieldValue.length > 0)
				{
					char * value = currFieldValue.value;
					memcpy(buffer, value, currFieldValue.length);
					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{
						char * value = currField.defauleValue;
						memcpy(buffer, value, strlen(currField.defauleValue));
						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_DECIMAL)
			{
				char * buffer = (char*)bind[columnIndex].buffer + currField.precision * rowIndex;
				if(currFieldValue.length > 0)
				{
					char * value = currFieldValue.value;
					memcpy(buffer, value, currFieldValue.length);
					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{
						char * value = currField.defauleValue;
						memcpy(buffer, value, strlen(currField.defauleValue));
						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_TIME)
			{
				MYSQL_TIME * buffer = (MYSQL_TIME*)bind[columnIndex].buffer + sizeof(MYSQL_TIME) + rowIndex;
				if(currFieldValue.length > 0)
				{

					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{

						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_DATE)
			{
				short * buffer = (short*)bind[columnIndex].buffer;
				if(currFieldValue.length > 0)
				{

					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{

						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_DATETIME)
			{
				short * buffer = (short*)bind[columnIndex].buffer;
				if(currFieldValue.length > 0)
				{

					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{

						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_TIMESTAMP)
			{
				short * buffer = (short*)bind[columnIndex].buffer;
				if(currFieldValue.length > 0)
				{

					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{

						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_CHAR || mdt_datatype == MDT_BINARY)
			{
				char * buffer = (char*)bind[columnIndex].buffer + sizeof(currField.precision) * rowIndex;
				if(currFieldValue.length > 0)
				{

					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{

						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_VARCHAR || mdt_datatype == MDT_VARBINARY)
			{
				short * buffer = (short*)bind[columnIndex].buffer;
				if(currFieldValue.length > 0)
				{

					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{

						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_TINYBLOB || mdt_datatype == MDT_TINYTEXT)
			{
				short * buffer = (short*)bind[columnIndex].buffer;
				if(currFieldValue.length > 0)
				{

					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{

						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_BLOB || mdt_datatype == MDT_TEXT)
			{
				short * buffer = (short*)bind[columnIndex].buffer;
				if(currFieldValue.length > 0)
				{

					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{

						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_MEDIUMBLOB || mdt_datatype == MDT_MEDIUMTEXT)
			{
				short * buffer = (short*)bind[columnIndex].buffer;
				if(currFieldValue.length > 0)
				{

					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{

						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_LONGBLOB || mdt_datatype == MDT_LONGTEXT)
			{
				short * buffer = (short*)bind[columnIndex].buffer;
				if(currFieldValue.length > 0)
				{

					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{

						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}
			else if(mdt_datatype == MDT_BIT)
			{
				short * buffer = (short*)bind[columnIndex].buffer;
				if(currFieldValue.length > 0)
				{

					bind[columnIndex].is_null[rowIndex] = 0;
				}
				else
				{
					if(strlen(currField.defauleValue) > 0)
					{

						bind[columnIndex].is_null[rowIndex] = 0;
					}
					else
					{
						bind[columnIndex].is_null[rowIndex] = 1;
					}
				}
			}

		}
	}



	return RM;
}
