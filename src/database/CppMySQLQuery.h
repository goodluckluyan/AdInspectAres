//@file:CppMySQLQuery.h
//@brief: 包含类CppMySQLQuery。此类为外部引用。
//封装了数据库记录集的各种访问操作。
//@引用者:wangzhongping@oristartech.com
//dade:2012-07-12


////////////////////////////////////////////////////////////////////////////////
// CppMysql - A C++ wrapper around the mysql database library.
//
// Copyright (c) 2009 Rob Groves. All Rights Reserved. lizp.net@gmail.com
// 
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without a written
// agreement, is hereby granted, provided that the above copyright notice, 
// this paragraph and the following two paragraphs appear in all copies, 
// modifications, and distributions.
//
// IN NO EVENT SHALL THE AUTHOR BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
// PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
// EVEN IF THE AUTHOR HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE AUTHOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF
// ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". THE AUTHOR HAS NO OBLIGATION
// TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
// u can use it for anything, but u must show the source
// frome http://rainfish.cublog.cn
// by ben
// if u find some questions, please tell me with email
//
// V1.0		18/09/2009	-Initial Version for cppmysql
////////////////////////////////////////////////////////////////////////////////
#ifndef _CPP_MYSQL_QUERY
#define _CPP_MYSQL_QUERY

#ifdef WIN32
#include <my_global.h>
#endif

#include "mysql.h"
#include <string.h>
#include <stdio.h>

//typedef unsigned int u_int;
//typedef unsigned long u_long;
//typedef MYSQL*		DB_HANDLE;

class CppMySQLQuery
{
	friend class CppMySQL3DB;
public:

    CppMySQLQuery();

	//当执行拷贝构造函数后，括号里的类已经无效，不能再使用
    CppMySQLQuery(CppMySQLQuery& rQuery);

	//当执行赋值构造函数后，=右边的类已经无效，不能再使用
    CppMySQLQuery& operator=(CppMySQLQuery& rQuery);

    virtual ~CppMySQLQuery();

	u_long numRow();//多少行
    int numFields();//多少列

    int fieldIndex(const char* szField);
	//0...n-1列
    const char* fieldName(int nCol);

 //   const char* fieldDeclType(int nCol);
 //   int fieldDataType(int nCol);

	u_long seekRow(u_long offerset);

    int getIntField(int nField, int nNullValue=0);
    int getIntField(const char* szField, int nNullValue=0);

    double getFloatField(int nField, double fNullValue=0.0);
    double getFloatField(const char* szField, double fNullValue=0.0);

	//0...n-1列
    const char* getStringField(int nField, const char* szNullValue="");
    const char* getStringField(const char* szField, const char* szNullValue="");

    const unsigned char* getBlobField(int nField, int& nLen);
    const unsigned char* getBlobField(const char* szField, int& nLen);

    bool fieldIsNull(int nField);
    bool fieldIsNull(const char* szField);

    bool eof();

    void nextRow();

    void finalize();
    void close();

private:
	void freeRes();
    void checkVM();
private:
	MYSQL_RES*		_mysql_res;
	MYSQL_FIELD*	_field;
	MYSQL_ROW		_row;
    unsigned long			_row_count;
    unsigned int			_field_count;


};
#endif //_CPP_MYSQL_QUERY
