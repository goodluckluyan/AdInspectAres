#include <utility>
#include <sys/time.h>
#include "CompareEngine.h"
#include "utility/FileEx.h"
#include "utility/C_Time.h"

extern bool g_bAresQuit;
extern MyLogger g_compare_logwrite;
#define loginfo(strlog,...)    g_compare_logwrite.PrintLog(MyLogger::INFO,strlog,##__VA_ARGS__)
#define logerror(strlog,...)   g_compare_logwrite.PrintLog(MyLogger::ERROR,strlog,##__VA_ARGS__)
#define logdebug(strlog,...)   g_compare_logwrite.PrintLog(MyLogger::DEBUG,strlog,##__VA_ARGS__)
#define logfatal(strlog,...)   g_compare_logwrite.PrintLog(MyLogger::FATAL,strlog,##__VA_ARGS__)
bool comparepic(PICTUR_ITEM* first ,PICTUR_ITEM* second)
{
    return first->quantity>second->quantity;
}


CompareEngine::CompareEngine(int hallid,TempletManager *ptrTempletMgr)
{
    m_hallid = hallid;
    m_ptrTempletMgr = ptrTempletMgr;
    m_loopbuffer = NULL;
    m_threshold = 0.1;
}

CompareEngine::~CompareEngine()
{

}


/*******************************************************************************
* 函数名称：	SetBCFun
* 功能描述：	设置比对完成回调函数，
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
void CompareEngine::SetBCFun(void * ptr,BC_CompareDone ptrFun)
{
    m_ptrUser = ptr;
    m_ptrCompareDoneFun = ptrFun;
    return ;
}


/*******************************************************************************
* 函数名称：	Init
* 功能描述：	初始化，设置比对区域，数据库参数，
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
int CompareEngine::Init(std::string &dbip,std::string &dbuser,std::string &passwd,std::string dbname,
                        int port,C_Para::Rect &rect)
{

    m_threshold = C_Para::GetInstance()->m_WeightThreshold;
    m_rect = rect;
    if(m_CompareResultDB.open(dbip.c_str(),dbuser.c_str(),
            passwd.c_str(),"oristarmr",port) == -1)
    {
            printf(0,"mysql open failed!\n");
            return -1;
    }
    return 0;
}


/*******************************************************************************
* 函数名称：	Compare
* 功能描述：	创建比对线程开始比对
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
int CompareEngine::Compare(std::string &taskid,FrameBufferLoop *fbl)
{
    if(m_ptrTempletMgr == NULL)
    {
        return -1;
    }
    OS_Thread::Join(this);
    CreateSearchArea(fbl);
    m_curtaskid = taskid;
    Run();
    return 0;
}


/*******************************************************************************
* 函数名称：	CreateSearchArea
* 功能描述：	创建每个录播图的搜索模板的范围
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
int CompareEngine::CreateSearchArea(FrameBufferLoop *fbl)
{
    int num = fbl->GetFrameCount();
    loginfo("Inspect frame count:%d",num);
    m_loopbuffer = fbl;
    fbl->SetContol(BUFFER_CONTROL_READ);
    for(int i = 0 ;i < num; i++)
    {
        ptrSearchArea sa(new SearchArea);
        fbl->ReadFrameData(i,&sa->ptrAVFrame);
        m_mapInspectSearchArea[i] = sa;
    }
    return 0;
}

/*******************************************************************************
* 函数名称：	Routine
* 功能描述：	比对线程函数
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
int CompareEngine::Routine()
{
    m_ptrTempletMgr->GetAllTemplets(m_rawTemplet);
    if(m_rawTemplet.size()==0)
    {
        return 0;
    }

    // 创建匹配记录数组
    C_Para *para = C_Para::GetInstance();
    int nPicSum = 0;

    time_t costtm_s;
    time(&costtm_s);
    loginfo("Start Compare ......");
    for(int i = 0 ;i < m_rawTemplet.size();i++)
    {
        TEMPLET_ITEM * ptr = m_rawTemplet[i];
        nPicSum+=ptr->picture_list.size();
        TempletMatch tm;
        tm.ptrTemplet = ptr;
        std::sort(ptr->picture_list.begin(),ptr->picture_list.end(),comparepic);

        PICTURE_LIST::iterator picit = ptr->picture_list.begin();
        for(;picit != ptr->picture_list.end();picit++)
        {
            PICTUR_ITEM* pic = *picit;
            loginfo("Templet %s:%d featrue num:%d",tm.ptrTemplet->ad_fileName,
                    pic->picture_order,pic->quantity);
        }

        std::string videopath;
        if( para->m_match_store_path.rfind("/")!= para->m_match_store_path.size()-1)
        {
            videopath = para->m_match_store_path+"/";
        }
        else
        {
            videopath = para->m_match_store_path  ;
        }

        videopath = videopath + "Result/" + tm.ptrTemplet->uuid + "/" + m_curtaskid;
        tm.resultpath = videopath;
        tm.vediofilename = tm.ptrTemplet->ad_fileName;
        m_mapTempletMatch.insert(std::pair<std::string,TempletMatch>(ptr->uuid,tm));
    }
    loginfo("Templet count:%d ,Templet frame count:%d",m_rawTemplet.size(),nPicSum);

    // 一次比较
    int index = 0;
    std::map<int,ptrSearchArea>::iterator fit = m_mapInspectSearchArea.begin();
    for(;fit != m_mapInspectSearchArea.end();fit++)
    {
        index ++;
        loginfo("Fist Compare(Inspect frame:%d)",index);
        ptrSearchArea &ptrSA = fit->second;
        Image_compare(fit->first,ptrSA,true);

        if(g_bAresQuit)
        {
            m_ptrTempletMgr->DeleteTemplet_list(&m_rawTemplet);
            ((FrameBufferLoop*)m_loopbuffer)->SetContol(BUFFER_CONTROL_IDLE);
            return 0;
        }
    }

    // 输出第一次比对结果,并标注
    std::map<std::string,TempletMatch>::iterator fpit = m_mapTempletMatch.begin();
    for(;fpit != m_mapTempletMatch.end();fpit++)
    {
        TempletMatch&tm = fpit->second;
        loginfo("Ad Templet %s First Match:",tm.ptrTemplet->ad_fileName);
        int len = tm.vecMatch.size();
        for(int i = 0; i < len ;i++)
        {
            int nInspectIndex;
            int nInspectTimeStamp;
            int nTempletIndex;
            float fWeight;
            MatchItem &mi = tm.vecMatch[i];
            loginfo("Match success(i:%d ts:%d t:%d w:%f)",mi.nInspectIndex,mi.nInspectTimeStamp,
                    mi.nTempletIndex,mi.fWeight);

            // 匹配成功，对录播图的左右邻居进行标注，以便二次匹配
            int start = std::max(mi.nInspectIndex-mi.nTempletIndex,0)+1;
            int end = start+tm.ptrTemplet->picture_quantity;
            Label_inspect(start,end,tm.ptrTemplet);
        }
    }



    // 二次比较
    index = 0;
    std::map<int,ptrSearchArea>::iterator sit = m_mapInspectSearchArea.begin();
    for(;sit != m_mapInspectSearchArea.end();sit++)
    {
        index ++;
        loginfo("Second Compare(Inspect frame:%d)",index);
         ptrSearchArea &ptrSA = sit->second;
        Image_compare(sit->first,ptrSA,para->m_IsSpeedPriority);

        if(g_bAresQuit)
        {
            m_ptrTempletMgr->DeleteTemplet_list(&m_rawTemplet);
            ((FrameBufferLoop*)m_loopbuffer)->SetContol(BUFFER_CONTROL_IDLE);
            return 0;
        }
    }

    // 输出第二次比对结果
    std::map<std::string,TempletMatch>::iterator pit = m_mapTempletMatch.begin();
    for(;pit != m_mapTempletMatch.end();pit++)
    {
        TempletMatch&tm = pit->second;
        loginfo("Ad Templet %s Second Match:",tm.ptrTemplet->ad_fileName);
        int len = tm.vecMatch.size();
        for(int i = 0; i < len ;i++)
        {
            int nInspectIndex;
            int nInspectTimeStamp;
            int nTempletIndex;
            float fWeight;
            MatchItem &mi = tm.vecMatch[i];
            loginfo("Match success(i:%d ts:%d t:%d w:%f)",mi.nInspectIndex,mi.nInspectTimeStamp,
                    mi.nTempletIndex,mi.fWeight);
        }
    }


    // 按模板统计结果并定位录播位置
    SummaryResultsAndLocatorPo();

    // 保存图片
    std::map<std::string,TempletMatch>::iterator tit = m_mapTempletMatch.begin();
    for(;tit != m_mapTempletMatch.end();tit++)
    {
        TempletMatch&tm = tit->second;
        SaveInspectImage(tm);
    }



    // 回调函数
    if(NULL != m_ptrCompareDoneFun)
    {
        m_ptrCompareDoneFun(m_ptrUser,m_curtaskid,m_mapTempletMatch);
    }

    // 结果保存到数据库
    InsertResult_DB();

    // 释放模板空间
    m_ptrTempletMgr->DeleteTemplet_list(&m_rawTemplet);
    ((FrameBufferLoop*)m_loopbuffer)->SetContol(BUFFER_CONTROL_IDLE);

    // 释放监播搜索结构
    m_mapInspectSearchArea.clear();

    time_t costtm_e;
    time(&costtm_e);
    loginfo("Finish Compare (cost time:%d sec)......",costtm_e - costtm_s);

    return 0;
}

/*******************************************************************************
* 函数名称：	image_compare
* 功能描述：	对录播图按搜索范围进行比对
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
int CompareEngine::Image_compare(int index,ptrSearchArea inspect,bool bSpeedPriority)
{
    if(inspect==NULL)
    {
        return -1;
    }

    // 第一次比较,只比较特征点数最多的前两个

    if(inspect->area == ALL)
    {
        loginfo("Inspect Image %d:%s",index,"All");

        // 开辟内存空间并提取特征
        if(inspect->ptrfeature == NULL)
        {
            // 转换RGB
            ImgBuf buf(inspect->ptrAVFrame->data,inspect->ptrAVFrame->length,
                       inspect->ptrAVFrame->width,inspect->ptrAVFrame->height);

            int featruenum;
            m_featrueExtract.ExportFeature((char *)buf.m_pBGR24,buf.m_lsize,buf.m_nWidth,buf.m_nHeight,featruenum);
            if(featruenum==0)
            {
                return -2;
            }

            inspect->ptrfeature = new unsigned char[featruenum*FEATURE_SIZE];
            inspect->featrueNum = featruenum;
            inspect->bufsize = featruenum*FEATURE_SIZE;
            m_featrueExtract.GetFeatureBuffer((char*)inspect->ptrfeature,inspect->bufsize);
        }

        // 和所有模板进行比对
        std::map<std::string,TempletMatch>::iterator it = m_mapTempletMatch.begin();
        for(;it != m_mapTempletMatch.end();it++)
        {
            TempletMatch &tm = it->second;
            PICTURE_LIST::iterator lit = tm.ptrTemplet->picture_list.begin();
            int comparecnt=0;
            if(bSpeedPriority)
            {
                comparecnt = 2;
            }
            else
            {
                comparecnt = tm.ptrTemplet->picture_list.size();
            }

            for(int i = 0;lit != tm.ptrTemplet->picture_list.end()&& i<comparecnt ;i++,lit++)
            {
                PICTUR_ITEM * ptrPIC = *lit;
                int matchcnt;
                unsigned int width = (unsigned int)atoi(tm.ptrTemplet->dstVideoWidth);
                unsigned int height = (unsigned int)atoi(tm.ptrTemplet->dstVideoHeight);
                unsigned int owidth = (unsigned int)inspect->ptrAVFrame->width;
                unsigned int oheight = (unsigned int)inspect->ptrAVFrame->height;
                int fn = (int)inspect->featrueNum;
                m_featrueExtract.CompareFeature((char *)ptrPIC->addr,
                                                ptrPIC->length,
                                                width,
                                                height,
                                                ptrPIC->quantity,
                                                (char *)inspect->ptrfeature,
                                                inspect->bufsize,
                                                owidth,
                                                oheight,
                                                fn,
                                                matchcnt
                                                );

                float weight = static_cast<float>(matchcnt)/((ptrPIC->quantity+inspect->featrueNum)/2);
                loginfo("Compare [%s:%d] matchcnt:%d weight:%.2f threshold:%.2f",
                        tm.ptrTemplet->ad_fileName,ptrPIC->picture_order,matchcnt,weight,m_threshold);
                if(weight > m_threshold)
                {
                    // 记录匹配对
                    MatchItem mi;
                    mi.nInspectIndex = index;
                    mi.nInspectTimeStamp = inspect->ptrAVFrame->framenum;
                    mi.nTempletIndex = ptrPIC->picture_order;
                    mi.fWeight = weight;
                    tm.vecMatch.push_back(mi);
                }
            }

        }//for
    }//if(inspect->area == ALL)
    else if(inspect->area == SUBAREA)
    {
        loginfo("Inspect Image %d:%s",index,"SUBAREA");

        // 只对标注广告进行搜索
        std::list<_TEMPLET_ITEM*>::iterator it = inspect->m_lsSearchTempletPtr.begin();
        for(;it != inspect->m_lsSearchTempletPtr.end();it++)
        {
            _TEMPLET_ITEM* ptrTemplet = *it;
            loginfo("......Compare inspect image %d - %s",index,ptrTemplet->ad_fileName);
            PICTURE_LIST::iterator lit = ptrTemplet->picture_list.begin();
            for(int i = 0;lit != ptrTemplet->picture_list.end()&& i<2 ;i++,lit++)
            {
                PICTUR_ITEM * ptrPIC = *lit;
                int matchcnt;
                unsigned int width =  (unsigned int)atoi(ptrTemplet->dstVideoWidth);
                unsigned int height = (unsigned int)atoi(ptrTemplet->dstVideoHeight);
                unsigned int owidth  = (unsigned int)inspect->ptrAVFrame->width;
                unsigned int oheight =(unsigned int)inspect->ptrAVFrame->height;
                int fn = (int)inspect->featrueNum;
                m_featrueExtract.CompareFeature((const char *)ptrPIC->addr,
                                                ptrPIC->length,
                                                width,
                                                height,
                                                ptrPIC->quantity,
                                                (const char *)inspect->ptrfeature,
                                                inspect->bufsize,
                                                owidth,
                                                oheight,
                                                fn,
                                                matchcnt
                                                );

                float weight = matchcnt/((ptrPIC->quantity+inspect->featrueNum)/2);
                loginfo("Compare [%s:%d] matchcnt:%d weight:%.2f threshold:%.2f",
                        ptrTemplet->ad_fileName,ptrPIC->picture_order,matchcnt,weight,m_threshold);

                if(weight > m_threshold)
                {
                    std::map<std::string,TempletMatch>::iterator fit = m_mapTempletMatch.find(ptrTemplet->uuid);
                    if(fit != m_mapTempletMatch.end())
                    {
                        TempletMatch & tm = fit->second;

                        // 记录匹配对
                        MatchItem mi;
                        mi.nInspectIndex = index;
                        mi.nInspectTimeStamp = inspect->ptrAVFrame->framenum;
                        mi.nTempletIndex = ptrPIC->picture_order;
                        mi.fWeight = weight;
                        tm.vecMatch.push_back(mi);
                    }
                }
            }
        }

    }
    return 0;
}

/*******************************************************************************
* 函数名称：	Label_inspect
* 功能描述：	对录播图的邻居进行标注
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
int CompareEngine::Label_inspect(int start,int end,_TEMPLET_ITEM* lable)
{
    for(int i = start ;i<=end;i++)
    {
        std::map<int,ptrSearchArea>::iterator fit =  m_mapInspectSearchArea.find(i);
        if(fit!= m_mapInspectSearchArea.end())
        {
            ptrSearchArea &ptr = fit->second;
            ptr->m_lsSearchTempletPtr.push_back(lable);
            ptr->area = SUBAREA;
        }
    }
}

/*******************************************************************************
* 函数名称：	SummaryResultsAndLocatorPo
* 功能描述：	按模板统计结果并定位录播位置
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
int CompareEngine::SummaryResultsAndLocatorPo()
{
    int nRet = 0;
    std::map<std::string,TempletMatch>::iterator it = m_mapTempletMatch.begin();
    for(; it != m_mapTempletMatch.end();it++)
    {
        TempletMatch &tm = it->second;
        if(tm.vecMatch.size() == 0)
        {
            continue;
        }

        Location loc;

        // 计算录播位置和模板位置差值
        for(int i = 0;i< (int)tm.vecMatch.size();++i)
        {
            MatchItem &Imi = tm.vecMatch[i];
            Relation rea;
#if 1
            if(Imi.nInspectIndex < Imi.nTempletIndex)
            {
                continue;
            }
#endif
            rea.luzhiPo = Imi.nInspectIndex;
            //std::cout<<"mobanName:"<< fg.m_name << "luzhiPo:"<< rea.luzhiPo << "-------------------"<< Imi.inspect_index<<std::endl;
            rea.mobanPo = Imi.nTempletIndex;
            //std::cout<<"mobanName:"<< fg.m_name << "mobanPo:"<< rea.mobanPo << "-------------------"<< Imi.module_index<<std::endl;
            rea.space   = abs(Imi.nInspectIndex-Imi.nTempletIndex);
            //std::cout << "Imi.weight"<< Imi.weight<<std::endl;
            //std::cout << "rea.space"<<rea.space<<std::endl;
            rea.luzhi_ts = Imi.nInspectTimeStamp;
            loc.spaces.push_back(rea);
        }

        // 统计差值相同的数量
        int n = 0;
        for(int a = 0;a < (int)loc.spaces.size();++a)
        {
            n = 0;
            //std::cout << "loc.demo.size()"<<loc.demo.size()<<std::endl;
            int nTemp = loc.spaces[a].space;
            for(int b = 0;b < loc.demo.size();++b)
            {
                if(nTemp == loc.demo[b].space)
                {
                    loc.demo[b].spaceCount++;
                    n = 1;
                }
            }

            if(n == 0)
            {
                SpaceRelation t ;
                t.space = nTemp;
                t.spaceCount = 1;
                t.ratio = 0;
                loc.demo.push_back(t);
            }
        }


        // 差值相同的数量的比值
        for(int q = 0;q <(int)loc.demo.size();++q)
        {
            //std::cout << "loc.demo[q].spaceCount:"<<loc.demo[q].spaceCount<<std::endl;
            //std::cout << "loc.spaces.size():"<<loc.spaces.size()<<std::endl;
            loc.demo[q].ratio = (float)loc.demo[q].spaceCount/loc.spaces.size();
            //std::cout << "loc.demo[q].ratio"<< loc.demo[q].ratio<<std::endl;
        }

        // 分配
        for(int i = 0;i<(int)loc.spaces.size();++i)
        {
            Relation rea = loc.spaces[i];
            for(int n =0;n < loc.demo.size();n++)
            {
                if(loc.demo[n].space == rea.space)
                {
                    loc.spaces[i].ratio = loc.demo[n].ratio;

                }
            }
        }

        // 加匹配系数
        for(int i = 0;i<(int)loc.spaces.size();++i)
        {
            for(int q = 0;q < (int)tm.vecMatch.size();q++)
            {
                if((tm.vecMatch[q].nInspectIndex == loc.spaces[i].luzhiPo)&&(tm.vecMatch[q].nTempletIndex == loc.spaces[i].mobanPo))
                {
                    //std::cout << "loc.spaces[i].luzhiPo:"<<loc.spaces[i].luzhiPo<<std::endl;
                    //std::cout << "loc.spaces[i].mobanPo"<< loc.spaces[i].mobanPo<<std::endl;
                    //std::cout << "loc.spaces[i].space"<< loc.spaces[i].space<<std::endl;
                    //std::cout << "loc.spaces[i].ratio"<<loc.spaces[i].ratio<<std::endl;
                    //std::cout << "fg.m_match.size():"<<fg.m_match.size()<<std::endl;
                    loc.spaces[i].ratio += tm.vecMatch[q].fWeight;
                    //std::cout << "i = "<<i<<std::endl;
                    //std::cout << "loc.spaces[i].ratio:"<<loc.spaces[i].ratio<<std::endl;
                }
            }
        }

        // 找到匹配系数最大的匹配项
        float f =loc.spaces[0].ratio;
        int g = 0;
        for(int i = 0;i< (int)loc.spaces.size();++i)
        {
            if(f < loc.spaces[i].ratio)
            {
                f = loc.spaces[i].ratio;
                g = i;

            }
        }

        // 定位
        std::cout<< "luzhiPo:"<<loc.spaces[g].luzhiPo<<"    "<< "mobanPo:"<< loc.spaces[g].mobanPo<<"  "<<"ratio:"<<loc.spaces[g].ratio<<std::endl;
        //std::cout << "loc.spaces[g].ratio"<<loc.spaces[g].ratio<<std::endl;
        tm.inspect_index_start = loc.spaces[g].luzhiPo - loc.spaces[g].mobanPo + 1;
        tm.inspect_ts_start = loc.spaces[g].luzhi_ts;
        if(tm.inspect_index_start< 0)
        {
            tm.inspect_index_start = -1*tm.inspect_index_start;
        }
        tm.inspect_index_end = tm.inspect_index_start + tm.ptrTemplet->picture_quantity - 1;
        tm.inspect_ts_end = tm.inspect_ts_start + tm.ptrTemplet->picture_quantity - 1;
    }

    // 确定播放位序
    std::map<std::string,TempletMatch>::iterator tit = m_mapTempletMatch.begin();
    for(; tit != m_mapTempletMatch.end();tit++)
    {
        TempletMatch &tm = tit->second;
        std::map<std::string,TempletMatch>::iterator sit = m_mapTempletMatch.begin();
        for(; sit != m_mapTempletMatch.end();sit++)
        {
            TempletMatch &stm = it->second;
            if( tm.inspect_ts_start >= stm.inspect_ts_start )
            {
                tm.showorder++;
            }
        }
    }


    return nRet;
}

/*******************************************************************************
* 函数名称：	InsertResult_DB
* 功能描述：	结果入库
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
bool CompareEngine::InsertResult_DB()
{

    std::map<std::string,TempletMatch>::iterator it = m_mapTempletMatch.begin();
    for(;it != m_mapTempletMatch.end();it++)
    {
        TempletMatch&tm = it->second;
        if(tm.vecMatch.size()==0 || tm.inspect_index_start ==0)
        {
            continue;
        }

        C_Para *para = C_Para::GetInstance();
        char sql[1024]={'\0'};
        char buf[128]={'\0'};
        snprintf(buf,128,"%d-%s",m_hallid,tm.ptrTemplet->uuid);
        std::string  id = buf;

        C_Time curtm;
        curtm.setCurTime();
        std::string strCurtime;
        curtm.getTimeStr(strCurtime);

        struct timeval tv;
        gettimeofday(&tv, NULL);
        unsigned long msec =  tv.tv_sec*1000+tv.tv_usec/1000;

        std::string cinema_city = para->m_LocationInfo;

        std::string videopath;
        if(  tm.resultpath.rfind("/")!=  tm.resultpath.size()-1)
        {
            videopath =  tm.resultpath+"/";
        }
        else
        {
            videopath =  tm.resultpath  ;
        }

        std::string head = para->m_match_store_path;
        if( head.rfind("/")!= head.size()-1)
        {
            head += "/";
        }

        std::string vediofullpath,fixbasepath;
        fixbasepath = videopath.substr(vediofullpath.find(head)+head.size()+1);
        vediofullpath =fixbasepath + tm.vediofilename;
        std::string compareimgpath = fixbasepath;

        C_Time starttm,endtm;
        starttm.setTimeInt(tm.inspect_ts_start);
        endtm.setTimeInt(tm.inspect_ts_end);
        std::string strStart;
        std::string strEnd;
        starttm.getTimeStr(strStart);
        endtm.getTimeStr(strEnd);

         snprintf(sql,1024,"insert into "
                          "app_monitor(id,advert_id,create_time,sortIdx,advert_back,"
                           "advert_previous,cinema_city,cinema_name,videopath,compare_imgpath,hall_no,"
                           "inspect_end_time,inspect_order,inspec_start_time,monitor_status,imglen) "
                           "values(\"%s\",\"%s\",\"%s\",%u,\"%s\","
                           "\"%s\",\"%s\",\"%s\",\"%s\",%d,"
                           "\"%s\",%d,\"%s\",%d,%d)",
                  id.c_str(),tm.ptrTemplet->uuid,strCurtime.c_str(),msec,"",
                  "","北京","大红门",vediofullpath.c_str(),compareimgpath.c_str(),m_hallid,
                  strEnd.c_str(),tm.showorder,strStart.c_str(),1,tm.inspect_index_end-tm.inspect_index_start+1
                  );
         int nResult = m_CompareResultDB.execSQL(sql);
         if(nResult != -1)
         {
             loginfo("Save compare result successful!(%s)",sql);
             return true;
         }
         else
         {
             loginfo("Save compare result failed!(%s)",sql);
             return false;
         }

    }




}

/*******************************************************************************
* 函数名称：	SaveInspectImage
* 功能描述：	保存比对成功的图片
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
bool CompareEngine::SaveInspectImage(TempletMatch &tm)
{
    for(int i = tm.inspect_index_start ;i<=tm.inspect_index_end;i++)
    {
        std::map<int,ptrSearchArea>::iterator fit =  m_mapInspectSearchArea.find(i);
        if(fit!= m_mapInspectSearchArea.end())
        {
            ptrSearchArea &ptr = fit->second;

            // YUV转换RGB并保存
            ImgBuf Img(ptr->ptrAVFrame->data,ptr->ptrAVFrame->length,
                       ptr->ptrAVFrame->width,ptr->ptrAVFrame->height);

            // 图片路径和文件命名方式：结果目录+模板uuid(广告表id)+taskid(hallid_cpos_start_duration)
            // +name(hallid+广告名称+开始时间+序号)
            char name[256]={'\0'};
            snprintf(name,256,"%d-%d.bmp",tm.inspect_ts_start,i);

            std::string savepath = tm.resultpath;
            if(savepath.rfind("/")!=savepath.size()-1)
            {
                savepath+="/";
            }
            std::string SavePath = savepath + std::string(name);
            CFileEx::CreateFolderForFile(SavePath.c_str());
            Img.Savebmp(SavePath);

        }
    }
}

