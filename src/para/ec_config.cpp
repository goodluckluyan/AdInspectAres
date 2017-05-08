//@file:ec_config.cpp
//@brief: ec_config����ʵ�֡�
//@������:wangzhongping@oristartech.com
//dade:2012-07-12


#include "ec_config.h"

ec_config::ec_config()
{
}
ec_config::~ec_config()
{
}
int ec_config::writevalue( const char *psec, const char *pkey, const char *pvalue, const char *ppath )
{
	int iret = 0;
	//char ctmp[MAX_BUFF_LEN];
	string stmp = "";
	string snewline = "\n";
	ofstream ofs;
	CFG_FILE cfg_file;
	int icntline = 0;
	int iindexline = 0;
	char ckey[MAX_BUFF_LEN];
	char cvalue[MAX_BUFF_LEN];

	if( NULL == psec ||
		NULL == ppath)
	{
		return -1;
	}

	iret = readfinfo(ppath,cfg_file,icntline);

	if( -1 == iret )
	{
		ofs.open(ppath,ios_base::out | ios_base::app);
	}
	else
	{
		ofs.open(ppath,ios_base::out);
	}


	if( NULL == ofs)
	{
		cout<<"open failed"<<endl;
		return -1;
	}

	/// ������ֵ��ؼ�ֵ

	if( NULL == psec )
	{
		ofs.close();
		return 0;	
	}

	trim(psec,cvalue,MAX_BUFF_LEN);

	if( 0 == strlen(cvalue) )
	{
		ofs.close();
		return 0;
	}


	unsigned int ipos_sec = std::string::npos;
	unsigned int ipos_key = std::string::npos;

	if(cfg_file.cfg_secs.size() > 0)
	{
		for( unsigned int iindex_sec = 0; iindex_sec < cfg_file.cfg_secs.size(); iindex_sec++ )
		{
			if( std::string::npos != cfg_file.cfg_secs[iindex_sec].ssec.find(psec) )
			{
				ipos_sec = iindex_sec;

				if( NULL == pkey )
				{
					break;
				}
				trim(pkey,ckey,MAX_BUFF_LEN);
				if( 0 == strlen(ckey) )
				{
					break;
				}

			}

			if( cfg_file.cfg_secs[iindex_sec].cfg_infos.size() > 0 )
			{
				for( unsigned int iindex_key = 0; 
					iindex_key < cfg_file.cfg_secs[iindex_sec].cfg_infos.size(); iindex_key++ )
				{
					if( CFG_KEY == cfg_file.cfg_secs[iindex_sec].cfg_infos[iindex_key].cfg_type)
					{
						if( std::string::npos != ipos_sec &&
							std::string::npos != cfg_file.cfg_secs[iindex_sec].cfg_infos[iindex_key].skeyname.find(pkey) )
						{
							ipos_key = iindex_key;
							break;
						}

					}
				}		
			}

			if(std::string::npos != ipos_sec &&
				std::string::npos != ipos_key )
			{
				break;
			}
		}
	}

	/// �����ֵ �ؼ�ֵ
	if( std::string::npos == ipos_sec)
	{
		if(NULL != pkey &&
			NULL != pvalue )
		{
			trim(pkey,ckey,MAX_BUFF_LEN);
			trim(pvalue,cvalue,MAX_BUFF_LEN);

			if( 0 != strlen(ckey) && 
				0 != strlen(cvalue) )
			{
				CFG_SECTION cfg_sec;
				CFG_INFO cfg_info;

				cfg_sec.ssec = "[";
				cfg_sec.ssec += psec;
				cfg_sec.ssec += "]";

				cfg_info.cfg_type = CFG_KEY;
				cfg_info.skeyname = pkey;
				cfg_info.skeyvalue = pvalue;

				cfg_sec.cfg_infos.push_back(cfg_info);
				cfg_file.cfg_secs.push_back(cfg_sec);

				icntline += 2;
			}
		}
	}
	else
	{
		if( NULL == pkey )
		{
			vector<CFG_SECTION>::iterator it = cfg_file.cfg_secs.begin();
			it += ipos_sec;

			icntline -= it->cfg_infos.size() + 1;

			cfg_file.cfg_secs.erase(it);
		}
		else
		{
			trim(pkey,ckey,MAX_BUFF_LEN);

			if( 0 == strlen(ckey) )
			{
				vector<CFG_SECTION>::iterator it = cfg_file.cfg_secs.begin();
				it += ipos_sec;
	
				icntline -= it->cfg_infos.size() + 1;

				cfg_file.cfg_secs.erase(it);
			}
			else
			{
				if( NULL == pvalue )
				{
					if( std::string::npos != ipos_key )
					{
						vector<CFG_INFO>::iterator it = cfg_file.cfg_secs[ipos_sec].cfg_infos.begin();
						it += ipos_key;
						cfg_file.cfg_secs[ipos_sec].cfg_infos.erase(it);

						icntline -= 1;
					}
				}
				else
				{
					trim(pvalue,cvalue,MAX_BUFF_LEN);

					if( 0 == strlen(cvalue) )
					{
						if( std::string::npos != ipos_key )
						{
							vector<CFG_INFO>::iterator it = cfg_file.cfg_secs[ipos_sec].cfg_infos.begin();
							it += ipos_key;
							cfg_file.cfg_secs[ipos_sec].cfg_infos.erase(it);

							icntline -= 1;
						}
					}
					else
					{
						if( std::string::npos == ipos_key )
						{
							CFG_INFO cfg_info;

							cfg_info.cfg_type = CFG_KEY;
							cfg_info.skeyname = pkey;
							cfg_info.skeyvalue = pvalue;

							cfg_file.cfg_secs[ipos_sec].cfg_infos.push_back(cfg_info);

							icntline += 1;
						}
						else
						{
							cfg_file.cfg_secs[ipos_sec].cfg_infos[ipos_key].skeyvalue = pvalue;
						}
					}
				}
			}
		}
	}


	/// д����
	if(cfg_file.cfg_infos.size() > 0)
	{
		for( vector<CFG_INFO>::iterator it = cfg_file.cfg_infos.begin();
			it != cfg_file.cfg_infos.end(); it++)
		{
			switch(it->cfg_type)
			{
			case CFG_COMMENT:
				ofs.write(it->scomment.c_str(),it->scomment.length());
				iindexline++;

				if( iindexline != icntline)
				{
					ofs.write(snewline.c_str(),snewline.length());
				}

				break;
			case CFG_BLANK:
				ofs.write(it->sblank.c_str(),it->sblank.length());
				iindexline++;

				if( iindexline != icntline)
				{
					ofs.write(snewline.c_str(),snewline.length());
				}

				break;
			case CFG_INVALID:
				ofs.write(it->sinvalid.c_str(),it->sinvalid.length());
				iindexline++;

				if( iindexline != icntline)
				{
					ofs.write(snewline.c_str(),snewline.length());
				}

				break;
				
			default:
				break;
			}
		}
	}

	if(cfg_file.cfg_secs.size() > 0)
	{
		for(vector<CFG_SECTION>::iterator it = cfg_file.cfg_secs.begin(); 
			it != cfg_file.cfg_secs.end(); it++ )
		{
			ofs.write(it->ssec.c_str(),it->ssec.length());
			iindexline++;
					
			if( iindexline != icntline)
			{
				ofs.write(snewline.c_str(),snewline.length());
			}

			if( it->cfg_infos.size() > 0 )
			{
				for( vector<CFG_INFO>::iterator itcfg = it->cfg_infos.begin();
					itcfg != it->cfg_infos.end(); itcfg++)
				{
					switch(itcfg->cfg_type)
					{
					case CFG_KEY:

						stmp = "";
						stmp = itcfg->skeyname + '=';
						stmp += itcfg->skeyvalue;

						if( "" != itcfg->scomment )
						{
							stmp += itcfg->scomment;
						}
						
						if( "" != itcfg->sblank )
						{
							stmp += itcfg->sblank;
						}

						ofs.write(stmp.c_str(),stmp.length());
						iindexline++;
			
						if( iindexline != icntline)
						{
							ofs.write(snewline.c_str(),snewline.length());
						}

						break;
					case CFG_COMMENT:

						ofs.write(itcfg->scomment.c_str(),itcfg->scomment.length());
						iindexline++;

						if( iindexline != icntline)
						{
							ofs.write(snewline.c_str(),snewline.length());
						}

						break;
					case CFG_BLANK:

						ofs.write(itcfg->sblank.c_str(),itcfg->sblank.length());
						iindexline++;

						if( iindexline != icntline)
						{
							ofs.write(snewline.c_str(),snewline.length());
						}

						break;
					case CFG_INVALID:

						ofs.write(itcfg->sinvalid.c_str(),itcfg->sinvalid.length());
						iindexline++;

						if( iindexline != icntline)
						{
							ofs.write(snewline.c_str(),snewline.length());
						}

						break;
						
						default:
							break;
					}
				
				}		
			}
		}
	}


	ofs.close();

	return 0;
}
int ec_config::readvalue( const char *psec, const char *pkey, char *pvalue, const char *ppath )
{
	int iret = 0;
	//char ctmp[MAX_BUFF_LEN];
	string stmp = "";
	CFG_FILE cfg_file;
	int ifind_key = 0;
	int ifind_sec = 0;
	int icntline = 0;

	char cbuf[MAX_BUFF_LEN];

	if( NULL == psec ||
		NULL == pkey ||
		NULL == pvalue ||
		NULL == ppath)
	{
		return -1;
	}

	iret = readfinfo(ppath,cfg_file,icntline);

	if(iret)
	{
		return -1;
	}

	if(cfg_file.cfg_secs.size() > 0)
	{
		for(vector<CFG_SECTION>::iterator it = cfg_file.cfg_secs.begin(); 
			it != cfg_file.cfg_secs.end(); it++ )
		{
			ifind_sec = 0;

			if( std::string::npos != it->ssec.find(psec) )
			{
				ifind_sec = 1;
			}

			ifind_key = 0;
			if( it->cfg_infos.size() > 0 )
			{
				for( vector<CFG_INFO>::iterator itcfg = it->cfg_infos.begin();
					itcfg != it->cfg_infos.end(); itcfg++)
				{
					if(CFG_KEY == itcfg->cfg_type )
					{
						if( std::string::npos != itcfg->skeyname.find(pkey) )
						{
							ifind_key = 1;
						}

						if( ifind_sec && ifind_key )
						{
							stmp = itcfg->skeyvalue;

							/// ȥ�����˿հ��ַ�
							trim(stmp.c_str(),cbuf,MAX_BUFF_LEN);
							sprintf(pvalue,"%s",cbuf);

							return 0;
						}

					}
				}		
			}
		}
	}
	

	return -1;
}

int ec_config::readfinfo( const char *ppath, CFG_FILE &cfg_file, int &icntline )
{
	int iret = 0;
	int iindexsec = -1;
	string strline;
	ifstream ifs;

	icntline = 0;

	if( NULL == ppath )
	{
		perror("path is NULL\n");
		return -1;
	}
	
	ifs.open( ppath, ios::in );

	if( NULL == ifs )
	{
		perror("file is not exist\n");
		return -1;
	}

	while( !ifs.eof() )
	{ 
		CFG_INFO cfg_info;
		getline(ifs,strline);
		
		icntline++;
		//if(strline == "")
		//{
		//	continue;
		//}
		//cout<<strline<<endl;

		iret = getkey(strline.c_str(),cfg_info);
		if( 1 == iret )
		{
			if( -1 == iindexsec)
			{
				cfg_info.cfg_type = CFG_INVALID;
				cfg_info.sinvalid = strline;
				cfg_info.sblank = "";
				cfg_info.scomment = "";
				cfg_info.skeyname = "";
				cfg_info.skeyvalue = "";
				cfg_info.ssec = "";

				cfg_file.cfg_infos.push_back(cfg_info);

				//cout<<"		it's a invalid string"<<endl;
			}
			else
			{
				cfg_file.cfg_secs[iindexsec].cfg_infos.push_back(cfg_info);

				//cout<<"		it's a key string"<<endl;
			}

			continue;
		}

		iret = getsection(strline.c_str(),cfg_info);
		if(iret)
		{
			CFG_SECTION cfg_sec;
			cfg_sec.ssec = strline;

			cfg_file.cfg_secs.push_back(cfg_sec);

			iindexsec++;

			//cout<<"		it's a section string"<<endl;

			continue;
		}

		iret = isblank(strline.c_str());
		if( 1 == iret )
		{
			cfg_info.cfg_type = CFG_BLANK;
			cfg_info.sblank = strline;
			cfg_info.scomment = "";
			cfg_info.skeyname = "";
			cfg_info.skeyvalue = "";
			cfg_info.ssec = "";
			cfg_info.sinvalid = "";

			if( -1 == iindexsec )
			{
				cfg_file.cfg_infos.push_back(cfg_info);
			}
			else
			{
				cfg_file.cfg_secs[iindexsec].cfg_infos.push_back(cfg_info);
			}

			//cout<<"		it's a blank string"<<endl;
			continue;
		}


		iret = iscomment(strline.c_str());
		if(iret)
		{
			cfg_info.cfg_type = CFG_COMMENT;
			cfg_info.scomment = strline;
			cfg_info.sblank = "";
			cfg_info.skeyname = "";
			cfg_info.skeyvalue = "";
			cfg_info.ssec = "";
			cfg_info.sinvalid = "";

			if( -1 == iindexsec )
			{
				cfg_file.cfg_infos.push_back(cfg_info);
			}
			else
			{
				cfg_file.cfg_secs[iindexsec].cfg_infos.push_back(cfg_info);
			}


			//cout<<"		it's a comment string"<<endl;
			continue;

		}

		cfg_info.cfg_type = CFG_INVALID;
		cfg_info.sinvalid = strline;
		cfg_info.sblank = "";
		cfg_info.scomment = "";
		cfg_info.skeyname = "";
		cfg_info.skeyvalue = "";
		cfg_info.ssec = "";

		if( -1 == iindexsec )
		{
			cfg_file.cfg_infos.push_back(cfg_info);
		}
		else
		{
			cfg_file.cfg_secs[iindexsec].cfg_infos.push_back(cfg_info);
		}

		//cout<<"		it's a invalid string"<<endl;

	}
	ifs.close();

	if( cfg_file.cfg_secs.size() > 0 )
	{
		if( cfg_file.cfg_secs.size() > 0 )
		{
			int ilast_sec = cfg_file.cfg_secs.size() - 1;

			if( cfg_file.cfg_secs[ilast_sec].cfg_infos.size() > 0 )
			{
				vector<CFG_INFO>::iterator it = cfg_file.cfg_secs[ilast_sec].cfg_infos.begin();
				it += cfg_file.cfg_secs[ilast_sec].cfg_infos.size() - 1;

				while( CFG_BLANK == it->cfg_type )
				{
					cfg_file.cfg_secs[ilast_sec].cfg_infos.erase(it);
					icntline--;

					if( 0 == cfg_file.cfg_secs[ilast_sec].cfg_infos.size() )
					{
						break;
					}

					it = cfg_file.cfg_secs[ilast_sec].cfg_infos.begin();
					it += cfg_file.cfg_secs[ilast_sec].cfg_infos.size() - 1;
				}
			}
		}		
	}
	else
	{
		if( cfg_file.cfg_infos.size() > 0 )
		{
			vector<CFG_INFO>::iterator it = cfg_file.cfg_infos.begin();
			it += cfg_file.cfg_infos.size() - 1;

			while( CFG_BLANK == it->cfg_type )
			{
				cfg_file.cfg_infos.erase(it);
				icntline--;

				if( 0 == cfg_file.cfg_infos.size() )
				{
					break;
				}

				it = cfg_file.cfg_infos.begin();
				it += cfg_file.cfg_infos.size() - 1;
			}
		}
	}

	return 0;
}

int ec_config::getsection( const char *pbuf, CFG_INFO &cfg_info )
{
	int ilenbuf = 0;
	string strbuf = "";
	string str_split1 = "";
	string str_split2 = "";
	string str = "";

	int ipos_split1 = -1;
	int ipos_split2 = -1;
	int ipos_char = -1;

	//cout<<"judge secton"<<endl;
	//cout<<pbuf<<endl;

	/// �ж��Ƿ�Ϊ�εı�׼�Էָ��� '[' ']' ��Ϊ���ޣ����ַ����ֳ�3����,
	/// '[' ֮ǰ���ַ�������Ϊ�ջ�ո��ַ�
	/// ']' ֮����ַ�������Ϊ�ջ�ո��ַ�
	/// '['��']'֮��Ĳ��ֲ���Ϊ�ջ�����ո�

	if( NULL == pbuf )
	{
		cout<<"string point is NULL"<<endl;
		return -1;
	}

	ilenbuf = strlen(pbuf);
	strbuf = pbuf;

	/// ���ҷָ�����־ '[' ']'
	for( int i = 0; i < ilenbuf; i++ )
	{
		if( SEC_FLAG1 == pbuf[i] )
		{
			if( -1 == ipos_split1)
			{
				ipos_split1 = i; 
			}
		}

		if( SEC_FLAG2 == pbuf[i] )
		{
			if( -1 == ipos_split2)
			{
				ipos_split2 = i; 
			}
		}

		if( SEC_FLAG1 != pbuf[i] &&
			BLANK_FLAG1 != pbuf[i] &&
			BLANK_FLAG2 != pbuf[i] &&
			BLANK_FLAG3 != pbuf[i])
		{
			if( -1 == ipos_char )
			{
				ipos_char = i;
			}
		}

		if( -1 != ipos_char && 
			-1 != ipos_split1 &&
			-1 != ipos_split2)
		{
			break;
		}
	}

	if( -1 == ipos_split1 ||
		-1 == ipos_split2)
	{
		return 0;
	}

	if( -1 == ipos_char ||
		ipos_split1 > ipos_char ||
		ipos_split1 > ipos_split2 ||
		ipos_split1 + 1 == ipos_split2)
	{
		return 0;
	}

	/// �ж� '['��']'֮����ַ���,�ַ�������ȫΪ�ո񣬻��ÿո�ֿ�
	//char csec[255];
	//memset(csec,0,255);
	//memcpy(csec,&pbuf[ipos_split1+1],ipos_split2 - ipos_split1 -1);
	//cout<<csec<<endl;

	int ipos_start = -1; /// sec�ַ�����ʼλ��
	int ipos_end = -1;	 /// sec�ַ�������λ��
	int ipos_blank = -1; /// sec�ַ����г��ֿո�ı�־
	for( int i = ipos_split1 + 1; i < ipos_split2 ; i++ )
	{
		//cout<<pbuf[i]<<endl;

		if( (pbuf[i] > 96 && pbuf[i] < 123 ) ||  // Сд��ĸ
			(pbuf[i] > 64 && pbuf[i] < 91 ) ||	 // ��д��ĸ
			(pbuf[i] > 47 && pbuf[i] < 58 ) ||	// ����	
				'_' == pbuf[i])	  				// �»���
		{
			if( ipos_start == -1 )
			{
				ipos_start = i;
			}

			if( ipos_start != -1 )
			{
				ipos_end = i;
			}
		}

		if( BLANK_FLAG1 == pbuf[i] ||
			BLANK_FLAG2 == pbuf[i] ||
			BLANK_FLAG3 == pbuf[i])
		{
			if( -1 != ipos_start && 
				-1 == ipos_blank)
			{
				ipos_blank = i;
			}
		}
	}

	if( -1 != ipos_blank && ipos_end > ipos_blank )
	{
		return 0;
	}

	/// �ж�']'֮��������ж��Ƿ�Ϊע�ͻ�ո��ַ������

	if( ilenbuf != ipos_split2 + 1 )
	{
		if(!iscomment(&pbuf[ipos_split2 + 1]))
		{
			if(!isblank(&pbuf[ipos_split2+1]))
			{
				return 0;
			}
		}
	}

	/// ������Ϣ
	cfg_info.ssec = pbuf;
	cfg_info.cfg_type = CFG_SEC;

	cfg_info.sblank = "";
	cfg_info.skeyname = "";
	cfg_info.skeyvalue = "";
	cfg_info.scomment = "";
	cfg_info.sinvalid = "";

	return 1;
}
int ec_config::iscomment( const char *pbuf )
{
	int iret = 0;
	int ipos_char = -1;
	int ipos_comment = -1;
	int ilenbuf = 0;

	if( NULL == pbuf )
	{
		cout<<"string point is NULL"<<endl;
		return -1;
	}

	ilenbuf = strlen(pbuf);

	/// ����ע�ͱ�־ '#'
	for( int i = 0; i < ilenbuf; i++ )
	{
		if( COMM_FLAG == pbuf[i] )
		{
			if( -1 == ipos_comment)
			{
				ipos_comment = i; 
			}
		}
		if( COMM_FLAG != pbuf[i] &&
			BLANK_FLAG1 != pbuf[i] &&
			BLANK_FLAG2 != pbuf[i] &&
			BLANK_FLAG3 != pbuf[i])
		{
			if( -1 == ipos_char )
			{
				ipos_char = i;
			}
		}

		if( -1 != ipos_char && 
			-1 != ipos_comment)
		{
			break;
		}
	}

	if( -1 != ipos_comment )
	{
		if( -1 == ipos_char ||
			ipos_char > ipos_comment)
		{
			//cout<<"it's a comment"<<endl;
			return 1;
		}
	}

	return iret;
}
int ec_config::getkey( const char *pbuf, CFG_INFO &cfg_info )
{
	int iret = 0;
	int ilenbuf = 0;
	int ipos_eq = -1;
	char ctmp[MAX_BUFF_LEN];

	if( NULL == pbuf )
	{
		cout<<"string point is NULL"<<endl;
		return -1;
	}

	ilenbuf = strlen(pbuf);

	/// ���ҹؼ�ֵ��־ '=',�Ե�һ���Ⱥų���λ��Ϊ׼,���ַ�����Ϊ������
	/// �Ⱥ�֮ǰΪ �ؼ�ֵ �����֣��ؼ�ֵ������֮ǰ����Ϊ�ջ�ո��ַ���
	/// �Ⱥ�֮��Ϊ �ؼ�ֵ ��ȡֵ, �ؼ�ֵ��ȡֵ֮����ַ�������Ϊ�ո��ע��

	for( int i = 0; i < ilenbuf; i++ )
	{
		if( KEYVALUE_FLAG == pbuf[i])
		{
			ipos_eq = i;
		}
	}

	if( -1 == ipos_eq ||
		0 == ipos_eq)
	{
		return 0;
	}

	/// �жϵȺ�֮ǰ���ַ����Ƿ����Ҫ��
	int ipos_start = -1; /// key�ַ���������ʼλ��
	int ipos_end = -1;	 /// key�ַ������ֽ���λ��
	int ipos_blank = -1; /// key�ַ��������г��ֿո�ı�־


	for( int i = 0; i < ipos_eq; i++)
	{
		//cout<<pbuf[i]<<endl;

		if( (pbuf[i] > 96 && pbuf[i] < 123 ) ||  // Сд��ĸ
			(pbuf[i] > 64 && pbuf[i] < 91 ) ||	 // ��д��ĸ
			(pbuf[i] > 47 && pbuf[i] < 58 ) ||	// ����	
				'_' == pbuf[i])	  				// �»���
		{
			if( ipos_start == -1 )
			{
				ipos_start = i;
			}

			if( ipos_start != -1 )
			{
				ipos_end = i;
			}
		}

		if( BLANK_FLAG1 == pbuf[i] ||
			BLANK_FLAG2 == pbuf[i] ||
			BLANK_FLAG3 == pbuf[i])
		{
			if( -1 != ipos_start && 
				-1 == ipos_blank)
			{
				ipos_blank = i;
			}
		}
	}

	if( -1 == ipos_start )
	{
		return 0;
	}

	if( -1 != ipos_blank && ipos_end > ipos_blank )
	{
		return 0;
	}

	/// �жϵȺ�֮����ַ����Ƿ����Ҫ��

	/// �Ƿ�Ϊ�հ���
	iret = isblank(&pbuf[ipos_eq+1]);
	if( iret )
	{
		return 0;
	}

	/// �Ƿ�Ϊע��
	iret = iscomment(&pbuf[ipos_eq+1]);
	if( iret )
	{
		return 0;
	}

	int ipos_start2 = -1; /// key�ַ���ȡֵ��ʼλ��
	int ipos_end2 = -1;	 /// key�ַ���ȡֵ����λ��
	int ipos_eof = -1;	///  key�ַ���ȡֵ������־

	for( int i = ipos_eq + 1; i < ilenbuf; i++)
	{
		//cout<<pbuf[i]<<endl;

		if( (pbuf[i] > 96 && pbuf[i] < 123 ) ||  // Сд��ĸ
			(pbuf[i] > 64 && pbuf[i] < 91 ) ||	 // ��д��ĸ
			(pbuf[i] > 47 && pbuf[i] < 58 ) ||	// ����	
				'_' == pbuf[i])	  				// �»���
		{
			if( ipos_start2 == -1 )
			{
				ipos_start2 = i;
			}

			if( ipos_start2 != -1 )
			{
				ipos_end2 = i;
			}
		}

		if( BLANK_FLAG1 == pbuf[i] || 
			COMM_FLAG == pbuf[i] ||
			BLANK_FLAG2 == pbuf[i] ||
			BLANK_FLAG3 == pbuf[i])
		{
			if( -1 != ipos_start2 )
			{
				ipos_eof = i;
				break;
			}
		}
	}


	int icommentflag = 0;
	if( -1 != ipos_eof )
	{
		/// �����������Ƿ��ǿո��ע��
		if(!iscomment(&pbuf[ipos_eof]))
		{
			if(!isblank(&pbuf[ipos_eof]))
			{
				return 0;
			}
		}
		else
		{
			icommentflag = 1;
		}
	}

	/// ���ؼ�ֵ��Ϣ
	cfg_info.cfg_type = CFG_KEY;
	cfg_info.ssec = "";
	cfg_info.sinvalid = "";
	/// �Ⱥ�֮ǰΪ�ؼ�ֵ����
	memset(ctmp,0,MAX_BUFF_LEN);
	memcpy(ctmp,&pbuf[0],ipos_eq);
	cfg_info.skeyname = ctmp;

	/// �Ⱥ�֮��Ϊ�ؼ�ֵȡֵ
	memset(ctmp,0,MAX_BUFF_LEN);
	memcpy(ctmp,&pbuf[ipos_eq + 1], ipos_end2 - ipos_eq);
	cfg_info.skeyvalue = ctmp;

	/// �ؼ�ֵȡֵ֮��Ϊע�ͻ�հ��ַ���
	if(  -1 != ipos_eof )
	{
		if(icommentflag)
		{
			cfg_info.scomment = &pbuf[ipos_eof];
		}
		else
		{
			cfg_info.sblank = &pbuf[ipos_eof];
		}
	}


	return 1;
}
int ec_config::isblank( const char *pbuf )
{
	int iret = 0;
	int ilenbuf = 0;
	int inum_blank = 0;

	if( NULL == pbuf )
	{
		cout<<"string point is NULL"<<endl;
		return -1;
	}

	ilenbuf = strlen(pbuf);
	if( 0 == ilenbuf )
	{
		return 1;
	}

	for(int i = 0; i < ilenbuf; i++ )
	{
		if( BLANK_FLAG1 == pbuf[i] || 
			BLANK_FLAG2 == pbuf[i] ||
			BLANK_FLAG3 == pbuf[i])
		{
			inum_blank++;
		}
	}

	if( inum_blank == ilenbuf )
	{
		return 1;
	}

	return iret;
}
int ec_config::trim( const char *pbuf,char *pdest,int ilenmax )
{
	int ilen = 0;
	int ipos_start = -1;
	int ipos_end = -1;



	if( NULL == pbuf  || NULL == pdest )
	{
		return -1;
	}

	ilen = strlen(pbuf);
	memset(pdest,0,ilenmax);

	if( 0 == ilen || 
		isblank(pbuf))
	{
		return 0;
	}

	for(int i = 0; i < ilen; i++)
	{
		if( 32 == pbuf[i] ||
			9 == pbuf[i] )
		{
			if( -1 != ipos_start )
			{
				break;
			}
		}
		else
		{
			if( -1 == ipos_start )
			{
				ipos_start = i;
			}

			ipos_end = i;
		}
	}

	ilen = ipos_end - ipos_start + 1;
	memcpy(pdest,&pbuf[ipos_start],ilen);

	return ilen;
}
