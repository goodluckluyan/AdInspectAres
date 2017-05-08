//@file:CppMySQLQuery.cpp
//@brief: CppMySQL3DB：功能实现。
//@author:wangzhongping@oristartech.com
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
//
// V1.0		18/09/2009	-Initial Version for cppmysql
////////////////////////////////////////////////////////////////////////////////

#include "CppMySQLQuery.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


CppMySQLQuery::CppMySQLQuery()
{
	_mysql_res = NULL;
	_field = NULL;
	_row = NULL;
	_row_count = 0;
	_field_count = 0;

}

CppMySQLQuery::CppMySQLQuery(CppMySQLQuery& rQuery)
{
	*this = rQuery;
}

CppMySQLQuery& CppMySQLQuery::operator=(CppMySQLQuery& rQuery)
{
	if ( this == &rQuery )
		return *this;

	//by wzp Add in 2012-06-27 
	//自动关闭记录级；
	 //freeRes();
	 //end

	_mysql_res = rQuery._mysql_res;
	_row = NULL;
	_row_count = 0;
	_field_count = 0;
	_field = NULL;

	if ( _mysql_res != NULL )
	{
		//定位游标位置到第一个位置
		mysql_data_seek(_mysql_res, 0);
		_row =  mysql_fetch_row( _mysql_res );
		_row_count = mysql_num_rows( _mysql_res ); 
 		//得到字段数量
		_field_count = mysql_num_fields( _mysql_res );
	}

	rQuery._mysql_res = NULL;
	rQuery._field = NULL;
	rQuery._row = NULL;
	rQuery._row_count = 0;
	rQuery._field_count = 0;

	return *this;

}

CppMySQLQuery::~CppMySQLQuery()
{
	freeRes();
}
void CppMySQLQuery::close()
{
		freeRes();
		return;
}
void CppMySQLQuery::freeRes()
{
	if ( _mysql_res != NULL )
	{
		mysql_free_result(_mysql_res);
		_mysql_res = NULL;
	}
}

u_long CppMySQLQuery::numRow()
{
	return _row_count;
}

int CppMySQLQuery::numFields()
{
	return _field_count;
}

u_long CppMySQLQuery::seekRow(u_long offerset)
{
	if ( offerset < 0 )
		offerset = 0;
	if ( offerset >= _row_count )
		offerset = _row_count -1;

	mysql_data_seek(_mysql_res, offerset);
	
	_row = mysql_fetch_row(_mysql_res);
	return offerset;
}

int CppMySQLQuery::fieldIndex(const char* szField)
{
	if ( NULL == _mysql_res )
		return -1;
	if ( NULL == szField )
		return -1;

	mysql_field_seek(_mysql_res, 0);//定位到第0列
	u_int i = 0;
	while ( i < _field_count )
	{
		_field = mysql_fetch_field( _mysql_res );
		if ( _field != NULL && strcmp(_field->name, szField) == 0 )//找到
			return i;

		i++;
	}

	return -1;
}

const char* CppMySQLQuery::fieldName(int nCol)
{
	if ( _mysql_res == NULL )
		return NULL;

	mysql_field_seek(_mysql_res, nCol);
	_field = mysql_fetch_field(_mysql_res);

	if ( _field != NULL )
		return _field->name;
	else
		return  NULL;
}

int CppMySQLQuery::getIntField(int nField, int nNullValue/*=0*/)
{
	if ( NULL == _mysql_res )
		return nNullValue;
	
	if ( nField + 1 > (int)_field_count )
		return nNullValue;
	
	if ( NULL == _row )
		return nNullValue;
	
	return atoi(_row[nField]);
}

int CppMySQLQuery::getIntField(const char* szField, int nNullValue/*=0*/)
{
	if ( NULL == _mysql_res || NULL == szField )
		return nNullValue;
	
	if ( NULL == _row )
		return nNullValue;

	const char* filed = getStringField(szField);

	if ( NULL == filed )
		return nNullValue;

	return atoi(filed);
}

const char* CppMySQLQuery::getStringField(int nField, const char* szNullValue/*=""*/)
{
	if ( NULL == _mysql_res )
		return szNullValue;

	if ( nField + 1 > (int)_field_count )
		return szNullValue;

	if ( NULL == _row )
		return szNullValue;
		
	if(_row[nField] == 0x0)
		return szNullValue;
		
	return _row[nField];
}
    
const char* CppMySQLQuery::getStringField(const char* szField, const char* szNullValue/*=""*/)
{
	if ( NULL == _mysql_res )
		return szNullValue;

	int nField = fieldIndex(szField);
	if ( nField == -1 )
		return szNullValue;
	
	return getStringField(nField);
}

double CppMySQLQuery::getFloatField(int nField, double fNullValue/*=0.0*/)
{
	const char* field = getStringField(nField);

	if ( NULL == field )
		return fNullValue;

	return atol(field);
}
 
double CppMySQLQuery::getFloatField(const char* szField, double fNullValue/*=0.0*/)
{
	const char* field = getStringField(szField);
	if ( NULL == field )
		return fNullValue;

	return atol(field);
}

void CppMySQLQuery::nextRow()
{
	if ( NULL == _mysql_res )
		return;

	_row = mysql_fetch_row(_mysql_res);
}

bool CppMySQLQuery::eof()
{
	if ( _row == NULL )
		return true;

	return false;
}
