//@file:ec_config.h
//@brief: ec_config��
//ec_config���ṩ�ļ����ʷ�װ��C_Para���� ����Ϊ�ⲿ���á�
//@������:wangzhongping@oristartech.com
//dade:2012-07-12


#ifndef _EC_CONFIG_H_
#define _EC_CONFIG_H_

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <string.h>
using namespace std;

#define SEC_FLAG1		91		// '['	
#define	SEC_FLAG2		93		// ']'
#define COMM_FLAG		35		// '#'
#define	KEYVALUE_FLAG	61		// '='
#define BLANK_FLAG1		32		// ' '
#define BLANK_FLAG2		9		// '	'
#define BLANK_FLAG3		13		// ����

//#define MAX_BUFF_LEN	255
#define MAX_BUFF_LEN	1024

typedef enum{CFG_SEC = 0,CFG_KEY = 1,CFG_COMMENT = 2,CFG_BLANK = 3,CFG_INVALID = 4}CFG_TYPE;

typedef struct _CFG_INFO
{
	string ssec;
	string skeyname;
	string skeyvalue;
	string scomment;
	string sblank;
	string sinvalid;
	CFG_TYPE cfg_type;
}CFG_INFO;

typedef struct _CFG_SECTION
{
	string ssec;
	vector<CFG_INFO>cfg_infos;
}CFG_SECTION;

typedef struct _CFG_FILE
{
	vector<CFG_INFO>cfg_infos;
	vector<CFG_SECTION>cfg_secs;
}CFG_FILE;



/// �ļ��ṹ���壺�ļ����� = <ע��> + <�հ���> + <����Ϣ1> + <����Ϣ2> + ....
/// ����Ϣ = �ؼ�ֵ + ע��

/// ע��:���� �� �ؼ�ֵ�� Ӧ��Ϊ��ĸ���ֻ��»��ߵ��������  ���������ע�ͻ�ո�
/// �ַ�'['��']'֮��Ϊ����
/// ������ע������,���ַ�'#'��Ϊע����ʼ��'#'֮ǰ�����пո񣬵���������������(���� �� �ؼ�ֵ������)

class ec_config
{
public:
	ec_config();
	~ec_config();

public:

	/// д��Ϣ
	int writevalue( const char *psec = NULL, const char *pkey = NULL, const char *pvalue = NULL, const char *ppath = NULL );
	/// ����Ϣ
	int readvalue( const char *psec = NULL, const char *pkey = NULL, char *pvalue = NULL, const char *ppath = NULL);

private:
	/// ��ȡ�ļ�����
	int readfinfo( const char *ppath, CFG_FILE &cfg_file, int &icntline );
	/// �Ƿ���ע������
	int iscomment( const char *pbuf );
	/// �Ƿ��Ƕ�����
	int getsection( const char *pbuf, CFG_INFO &cfg_info );
	/// �Ƿ��ǹؼ�ֵ
	int getkey( const char *pbuf, CFG_INFO &cfg_info );
	/// �Ƿ��ǿհ���
	int isblank( const char *pbuf );
	/// ȥ�����˵Ŀհ��ַ����ո���Ʊ��),�����ַ�������
	int trim( const char *pbuf,char *pdest,int ilenmax );

};

#endif
