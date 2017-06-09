#include <utility>
#include <sys/time.h>
#include "CompareEngine.h"
#include "utility/FileEx.h"
#include "utility/C_Time.h"
#include "utility/wordcodec.h"

extern bool g_bAresQuit;
extern MyLogger g_compare_logwrite;

#define loginfo(strlog,...)    g_compare_logwrite.PrintLog(MyLogger::INFO,strlog,##__VA_ARGS__)
#define logerror(strlog,...)   g_compare_logwrite.PrintLog(MyLogger::ERROR,strlog,##__VA_ARGS__)
#define logdebug(strlog,...)   g_compare_logwrite.PrintLog(MyLogger::DEBUG,strlog,##__VA_ARGS__)
#define logfatal(strlog,...)   g_compare_logwrite.PrintLog(MyLogger::FATAL,strlog,##__VA_ARGS__)



_TempletMatch::_TempletMatch()
{
    ptrTemplet = NULL;
    inspect_ts_start = 0;
    inspect_ts_end = 0;
    inspect_index_start = 0;
    inspect_index_end  = 0;
    showorder = 0;
}

_TempletMatch::~_TempletMatch()
{
//   logdebug("delete TempletMatch");
}

_SearchArea::_SearchArea()
{
    area = ALL;
    ptrAVFrame = NULL;
    bufsize = 0;
    featrueNum = 0;
    ptrfeature = NULL;
    width_hr = 0;
    height_hr = 0;

}

_SearchArea::~_SearchArea()
{
    if(ptrfeature!=NULL)
    {
        delete[] ptrfeature;
//        logdebug("delete [] ptrfeature");
    }
}

CompareEngine::CompareEngine(int hallid,TempletManager *ptrTempletMgr,
                             std::string city,std::string cinema_name,C_CS *pCS)
{
    m_hallid = hallid;
    m_ptrTempletMgr = ptrTempletMgr;
    m_loopbuffer = NULL;
    m_threshold = 0.1;
    m_City = city;
    m_CinemaName = cinema_name;
    m_pTempletMgrMutx = pCS;
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

    C_Para *para = C_Para::GetInstance();
    m_threshold = para->m_WeightThreshold;
    m_match_count_threshold = para->m_MatchCountThreshold;
    m_rect = rect;
    m_strDB_IP=dbip;
    m_strDB_User=dbuser;
    m_strPasswd=passwd;
    m_strDB_name=dbname;
    m_nPort = port;

    if(para->m_FeatrueType=="sift")
    {
        m_featrueExtract.InitModule(0);
    }
    else
    {
        m_featrueExtract.InitModule(1);
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
int CompareEngine::Compare(std::string &taskid,time_t tm,FrameBufferLoop *fbl)
{
    if(m_ptrTempletMgr == NULL)
    {
        return -1;
    }
    OS_Thread::Join(this);
    CreateSearchArea(fbl);
    m_curtaskid = taskid;
    m_curtask_start_tm = tm;
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
    for(int i = 1 ;i <= num; i++)
    {
        ptrSearchArea sa(new SearchArea);
        if(fbl->ReadFrameData(i-1,&sa->ptrAVFrame)==0)
        {
            m_mapInspectSearchArea[i] = sa;
        }
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

    C_Time tasktm;
    tasktm.setTimeInt(m_curtask_start_tm);

    std::string strCurTaskTm;
    tasktm.getTimeStr(strCurTaskTm);
    m_pTempletMgrMutx->EnterCS();
    m_ptrTempletMgr->GetAllTemplets(strCurTaskTm,m_rawTemplet);
    m_pTempletMgrMutx->LeaveCS();

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

        std::string tmpType = ptr->featrue_type;
        std::transform(tmpType.begin(),tmpType.end(),tmpType.begin(),std::towlower);
        if(para->m_FeatrueType!=tmpType)
        {
            logerror("Featrue type not match config file:%s<->%s(%s) templet type:%s,so this templet ignored",
                     para->m_FeatrueType.c_str(),ptr->ad_fileName,ptr->uuid,tmpType.c_str());
            continue;
        }

        int framecnt = ptr->picture_list.size();
        nPicSum += framecnt;
        TempletMatch tm;
        tm.ptrTemplet = ptr;

//      std::sort(ptr->picture_list.begin(),ptr->picture_list.begin()+framecnt/3,ComparePic());
//      std::sort(ptr->picture_list.begin()+framecnt/3,ptr->picture_list.begin()+2*framecnt/3,ComparePic());
//      std::sort(ptr->picture_list.begin()+2*framecnt/3,ptr->picture_list.end(),ComparePic());

        std::sort(ptr->picture_list.begin(),ptr->picture_list.begin()+framecnt/4,ComparePic());
        std::sort(ptr->picture_list.begin()+framecnt/4,ptr->picture_list.begin()+framecnt/2,ComparePic());
        std::sort(ptr->picture_list.begin()+framecnt/2,ptr->picture_list.begin()+3*framecnt/4,ComparePic());
        std::sort(ptr->picture_list.begin()+3*framecnt/4,ptr->picture_list.end(),ComparePic());

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
        tm.vediofilename = tm.ptrTemplet->uuid;
        m_mapTempletMatch.insert(std::pair<std::string,TempletMatch>(ptr->uuid,tm));
    }
    loginfo("Templet count:%d ,Templet frame count:%d",m_rawTemplet.size(),nPicSum);

    // 一次比较
    int index = 0;
    std::map<int,ptrSearchArea>::iterator fit = m_mapInspectSearchArea.begin();
    for(;fit != m_mapInspectSearchArea.end();fit++)
    {
        index ++;
        loginfo("Fist Compare(Inspect frame:%d/%d)",index,m_mapInspectSearchArea.size());
        ptrSearchArea &ptrSA = fit->second;
        Image_compare(fit->first,ptrSA,para->m_IsSpeedPriority);

        if(g_bAresQuit)
        {
            m_ptrTempletMgr->DeleteTemplet_list(&m_rawTemplet);
            ((FrameBufferLoop*)m_loopbuffer)->SetContol(BUFFER_CONTROL_IDLE);

            // 释放监播搜索和模板结构
            m_mapInspectSearchArea.clear();
            m_mapTempletMatch.clear();
            return 0;
        }
    }

    // 输出第一次比对结果,并标注
    int nMinIndex=m_mapInspectSearchArea.size();
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

            // 快速比较进行二次比较，所有标注一下
            if(para->m_IsSpeedPriority)
            {
                // 匹配成功，对录播图的左右邻居进行标注，以便二次匹配
                int start = std::max(mi.nInspectIndex-mi.nTempletIndex,0)+1;
                int end = start+tm.ptrTemplet->picture_quantity;
                Label_inspect(start,end,tm.ptrTemplet);

                if(start < nMinIndex)
                {
                    nMinIndex = start;
                }
            }
        }
    }


    // 快速比较进行二次按标注比较
    if(para->m_IsSpeedPriority)
    {
        // 对判断的前期空白区域标注为空
        for(int i = 1 ;i<nMinIndex;i++)
        {
            std::map<int,ptrSearchArea>::iterator fit =  m_mapInspectSearchArea.find(i);
            if(fit!= m_mapInspectSearchArea.end())
            {
                ptrSearchArea &ptr = fit->second;
                ptr->area = NOTHING;
                loginfo("lable inspect iamge %d is NOTHING",i);
            }
        }

        // 二次比较
        index = 0;
        std::map<int,ptrSearchArea>::iterator sit = m_mapInspectSearchArea.begin();
        for(;sit != m_mapInspectSearchArea.end();sit++)
        {
            index ++;
            loginfo("Second Compare(Inspect frame:%d/%d)",index,m_mapInspectSearchArea.size());
             ptrSearchArea &ptrSA = sit->second;
            Image_compare(sit->first,ptrSA);

            if(g_bAresQuit)
            {
                m_ptrTempletMgr->DeleteTemplet_list(&m_rawTemplet);
                ((FrameBufferLoop*)m_loopbuffer)->SetContol(BUFFER_CONTROL_IDLE);

                // 释放监播搜索和模板结构
                m_mapInspectSearchArea.clear();
                m_mapTempletMatch.clear();
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
    }

    // 匹配结果入库
    InsertMatch_DB();

    // 按模板统计结果并定位录播位置
    std::vector<suspicious_show> vecss;
    SummaryResultsAndLocatorPo(vecss);

    // 保存图片
    std::map<std::string,TempletMatch>::iterator tit = m_mapTempletMatch.begin();
    for(;tit != m_mapTempletMatch.end();tit++)
    {
        TempletMatch&tm = tit->second;
        if(tm.vecMatch.size()>0 /*&& tm.inspect_ts_start !=0*/)
        {
             SaveInspectImage(tm);
        }
    }

    // 结果保存到数据库
    InsertResult_DB();
    InsertSuspiciousShow_DB(vecss);

    // 回调函数
    if(NULL != m_ptrCompareDoneFun)
    {
        m_ptrCompareDoneFun(m_ptrUser,m_curtaskid,m_mapTempletMatch);
    }

    // 释放模板空间
    m_ptrTempletMgr->DeleteTemplet_list(&m_rawTemplet);
    ((FrameBufferLoop*)m_loopbuffer)->SetContol(BUFFER_CONTROL_IDLE);

    // 释放监播搜索和模板结构
    m_mapInspectSearchArea.clear();
    m_mapTempletMatch.clear();

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

            char *ptrbuf = buf.m_pBGR24;
            unsigned int bufsize = buf.m_lsize;
            unsigned int width = buf.m_nWidth;
            unsigned int height = buf.m_nHeight;
            if(m_rect.width()>0&&m_rect.height()>0)
            {
               ptrbuf = buf.SetHRRect(m_rect.left,m_rect.top,m_rect.right,m_rect.bottom);
               bufsize = buf.m_lsize_hr;
               width = m_rect.width();
               height = m_rect.height();
            }

            if(NULL == ptrbuf)
            {
                logerror("Inspect transfrom to RGB failed!");
                return -3;
            }

            int featruenum;
            m_featrueExtract.ExportFeature(ptrbuf,bufsize,width,height,featruenum);
            if(featruenum==0)
            {
                inspect->area = NOTHING;
                return -2;
            }

            if(featruenum>0)
            {
                inspect->ptrfeature = new unsigned char[featruenum*FEATURE_SIZE];
                inspect->featrueNum = featruenum;
                inspect->bufsize = featruenum*FEATURE_SIZE;
                inspect->width_hr = width;
                inspect->height_hr = height;
                m_featrueExtract.GetFeatureBuffer((char*)inspect->ptrfeature,inspect->bufsize);
            }
            else
            {
                loginfo("Inspect iamge [%d] No featrue.");
            }

        }

        // 和所有模板进行比对
        std::map<std::string,TempletMatch>::iterator it = m_mapTempletMatch.begin();
        for(;it != m_mapTempletMatch.end();it++)
        {
            TempletMatch &tm = it->second;

            PICTURE_LIST lsCompared;
            if(bSpeedPriority)
            {
                // 获取前半段和后半段的最大和次大值
                int comparecnt = 1;
                int nFrameCnt = tm.ptrTemplet->picture_list.size();
//                PICTURE_LIST::iterator lit = tm.ptrTemplet->picture_list.begin();
//                PICTURE_LIST::iterator olit = tm.ptrTemplet->picture_list.begin()+nFrameCnt/3;
//                PICTURE_LIST::iterator tlit = tm.ptrTemplet->picture_list.begin()+2*nFrameCnt/3;
//                for(int i = 0;lit != olit && i<comparecnt ;i++,lit++)
//                {
//                    lsCompared.push_back(*lit);
//                }

//                for(int i = 0;olit != tlit && i<comparecnt ;i++,olit++)
//                {
//                    lsCompared.push_back(*olit);
//                }

//                for(int i = 0;tlit != tm.ptrTemplet->picture_list.end() && i<comparecnt ;i++,olit++)
//                {
//                    lsCompared.push_back(*olit);
//                }

                PICTURE_LIST::iterator lit =  tm.ptrTemplet->picture_list.begin();
                PICTURE_LIST::iterator olit = tm.ptrTemplet->picture_list.begin()+nFrameCnt/4;
                PICTURE_LIST::iterator tlit = tm.ptrTemplet->picture_list.begin()+nFrameCnt/2;
                PICTURE_LIST::iterator qlit = tm.ptrTemplet->picture_list.begin()+3*nFrameCnt/4;
                for(int i = 0;lit != olit && i<comparecnt ;i++,lit++)
                {
                    lsCompared.push_back(*lit);
                }

                for(int i = 0;olit != tlit && i<comparecnt ;i++,olit++)
                {
                    lsCompared.push_back(*olit);
                }

                for(int i = 0;tlit != qlit && i<comparecnt ;i++,olit++)
                {
                    lsCompared.push_back(*olit);
                }

                for(int i = 0;qlit != tm.ptrTemplet->picture_list.end() && i<comparecnt ;i++,olit++)
                {
                    lsCompared.push_back(*olit);
                }


            }
            else
            {
                PICTURE_LIST::iterator lit = tm.ptrTemplet->picture_list.begin();
                for(;lit != tm.ptrTemplet->picture_list.end() ;lit++)
                {
                    lsCompared.push_back(*lit);
                }
            }


            PICTURE_LIST::iterator lit_compared = lsCompared.begin();
            for(;lit_compared != lsCompared.end() ;lit_compared++)
            {
                PICTUR_ITEM * ptrPIC = *lit_compared;
                float fpretest = static_cast<float>(inspect->featrueNum)/((ptrPIC->quantity+inspect->featrueNum)/2);
                if(fpretest<m_threshold ||inspect->featrueNum==0 || ptrPIC->quantity==0)
                {
                    loginfo("featrue too less not compare. [ %d(%d)-%s:%d(%d)]",index,inspect->featrueNum,
                            tm.ptrTemplet->ad_fileName,ptrPIC->picture_order,ptrPIC->quantity);
                    continue;
                }

                if(inspect->ptrfeature==NULL)
                {
                    continue;
                }

                int matchcnt;
                unsigned int width = (unsigned int)atoi(tm.ptrTemplet->dstVideoWidth);
                unsigned int height = (unsigned int)atoi(tm.ptrTemplet->dstVideoHeight);
                unsigned int owidth = inspect->width_hr;
                unsigned int oheight = inspect->height_hr;
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

                // 广告名有中文会乱码，则转换
//                CodeConverter conv("gb2312","utf-8");
//                char buf[256]={'\n'};
//                conv.convert(tm.ptrTemplet->ad_fileName,strlen(tm.ptrTemplet->ad_fileName),
//                             buf,256);
//                loginfo("Compare [%s:%d] matchcnt:%d weight:%.4f threshold:%.2f",
//                        tm.ptrTemplet->ad_fileName,ptrPIC->picture_order,matchcnt,weight,m_threshold);
                if(weight >= m_threshold && matchcnt > m_match_count_threshold)
                {
                    loginfo("Compare [%s:%d] matchcnt:%d weight:%.4f threshold:%.2f",
                            tm.ptrTemplet->ad_fileName,ptrPIC->picture_order,matchcnt,weight,m_threshold);

                    // 记录匹配对
                    MatchItem mi;
                    mi.nInspectIndex = index;
                    mi.nInspectTimeStamp = inspect->ptrAVFrame->framenum;
                    mi.nTempletIndex = ptrPIC->picture_order;
                    mi.fWeight = weight;
                    mi.nInspectFeatrue = inspect->featrueNum;
                    mi.nTempletFeatrue = ptrPIC->quantity;
                    mi.nMatchCount = matchcnt;
                    std::vector<MatchItem>::iterator mfit = std::find_if(tm.vecMatch.begin(),
                                                                         tm.vecMatch.end(),
                                                                         match_equals(mi));
                    if(mfit == tm.vecMatch.end())
                    {
                         tm.vecMatch.push_back(mi);
                    }

                }

            }

        }//for

        // 比对过的标记为NOTHING
        // inspect->area = NOTHING;

    }//if(inspect->area == ALL)
    else if(inspect->area == SUBAREA)
    {
        loginfo("Inspect Image %d:%s",index,"SUBAREA");

        // 只对标注广告进行搜索
        std::list<_TEMPLET_ITEM*>::iterator it = inspect->m_lsSearchTempletPtr.begin();
        for(;it != inspect->m_lsSearchTempletPtr.end();it++)
        {
            _TEMPLET_ITEM* ptrTemplet = *it;

//            CodeConverter conv("gb2312","utf-8");
//            char buf[256]={'\n'};
//            conv.convert(ptrTemplet->ad_fileName,strlen(ptrTemplet->ad_fileName),
//                         buf,256);

            loginfo("......Compare inspect image %d - %s",index,ptrTemplet->ad_fileName);
            PICTURE_LIST::iterator lit = ptrTemplet->picture_list.begin();
            for(;lit != ptrTemplet->picture_list.end() ;lit++)
            {
                PICTUR_ITEM * ptrPIC = *lit;
                float fpretest = static_cast<float>(inspect->featrueNum)/((ptrPIC->quantity+inspect->featrueNum)/2);
                if(fpretest<m_threshold||inspect->featrueNum==0 || ptrPIC->quantity==0)
                {
                    loginfo("featrue too less not compare. [ %d(%d)-%s:%d(%d)]",index,inspect->featrueNum,
                            ptrTemplet->ad_fileName,ptrPIC->picture_order,ptrPIC->quantity);
                    continue;
                }

                if(inspect->ptrfeature==NULL)
                {
                    continue;
                }


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

                float weight = static_cast<float>(matchcnt)/((ptrPIC->quantity+inspect->featrueNum)/2);
//                loginfo("Compare [%s:%d] matchcnt:%d weight:%.4f threshold:%.2f",
//                        ptrTemplet->ad_fileName,ptrPIC->picture_order,matchcnt,weight,m_threshold);

                if(weight >= m_threshold && matchcnt > m_match_count_threshold)
                {
                    loginfo("Compare [%s:%d] matchcnt:%d weight:%.4f threshold:%.2f",
                                           ptrTemplet->ad_fileName,ptrPIC->picture_order,matchcnt,weight,m_threshold);

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
                        mi.nInspectFeatrue = inspect->featrueNum;
                        mi.nTempletFeatrue = ptrPIC->quantity;
                        mi.nMatchCount = matchcnt;

                        std::vector<MatchItem>::iterator mfit = std::find_if(tm.vecMatch.begin(),
                                                                             tm.vecMatch.end(),
                                                                             match_equals(mi));
                        if(mfit == tm.vecMatch.end())
                        {
                             tm.vecMatch.push_back(mi);
                        }

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
            std::list<_TEMPLET_ITEM*>::iterator fit = std::find_if(ptr->m_lsSearchTempletPtr.begin(),ptr->m_lsSearchTempletPtr.end(),
                         shared_equals_raw<_TEMPLET_ITEM>(lable));
            if(fit == ptr->m_lsSearchTempletPtr.end())
            {
                ptr->m_lsSearchTempletPtr.push_back(lable);
                ptr->area = SUBAREA;
            }
        }
    }
}

/*******************************************************************************
* 函数名称：	SummaryResultsAndLocatorPo
* 功能描述：	按模板统计结果并定位录播位置
* 输入参数：
* 输出参数： vecss:可疑播放列表
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
int CompareEngine::SummaryResultsAndLocatorPo(std::vector<suspicious_show> &vecss)
{
    int nRet = 0;
    std::map<std::string,TempletMatch>::iterator it = m_mapTempletMatch.begin();
    for(; it != m_mapTempletMatch.end();it++)
    {
        TempletMatch &tm = it->second;
        if(tm.vecMatch.size() ==0)
        {
            continue;
        }

        // 只有一个匹配项则判断权重是否大于阈值的1.5倍，小于则认为误匹配
        if(tm.vecMatch.size() == 1)
        {
            MatchItem &Imi = tm.vecMatch[0];
            if(Imi.fWeight <= C_Para::GetInstance()->m_WeightThreshold*1.5)
            {
                continue;
            }
        }

//        float max;
//        CalcAvgMatchWeight(tm.vecMatch,max);
//        if(tm.vecMatch.size()<10 && max<0.01)
//        {
//            continue;
//        }

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
        if(loc.spaces.size()>0)
        {
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
            tm.inspect_ts_start = loc.spaces[g].luzhi_ts - loc.spaces[g].mobanPo + 1;
            if(tm.inspect_index_start< 0)
            {
                tm.inspect_index_start = -1*tm.inspect_index_start;
            }
            tm.inspect_index_end = tm.inspect_index_start + tm.ptrTemplet->picture_quantity - 1;
            tm.inspect_ts_end = tm.inspect_ts_start + tm.ptrTemplet->picture_quantity - 1;


        }

    }


    // 确定播放位序和前后广告
    std::vector<PAIR> vecTmp(m_mapTempletMatch.begin(),m_mapTempletMatch.end());
    std::sort(vecTmp.begin(),vecTmp.end(),CmpByValue());
    for(int i = 0 ;i < vecTmp.size();i++)
    {
        PAIR &p = vecTmp[i];
        TempletMatch &tm = p.second;
        if(tm.inspect_ts_start==0)
        {
            break;
        }

         std::map<std::string,TempletMatch>::iterator fit = m_mapTempletMatch.find(p.first);
         if(fit != m_mapTempletMatch.end())
         {
             TempletMatch &ftm = fit->second;
             ftm.showorder  = i+1;
             if(i-1>=0)
             {
                 PAIR &pre = vecTmp[i-1];
                 TempletMatch &pretm = pre.second;
                 if(pretm.vecMatch.size()!=0 && pretm.inspect_ts_start!=0)
                 {
                      ftm.backAd = pretm.ptrTemplet->ad_fileName;
                 }

             }

             if(i+1<vecTmp.size())
             {
                 PAIR &pre = vecTmp[i+1];
                 TempletMatch &pretm = pre.second;
                 if(pretm.vecMatch.size()!=0 && pretm.inspect_ts_start!=0)
                 {
                      ftm.preAd = pretm.ptrTemplet->ad_fileName;
                 }

             }
         }
    }


    // 计算广告间的时长，大于5秒则认为可疑播放
    C_Para *para = C_Para::GetInstance();
    int calc_cnt = vecTmp.size();
    for(int i = 1 ;i<calc_cnt;i++)
    {
        unsigned int interval;
        if(vecTmp[i-1].second.inspect_ts_start>0 && vecTmp[i].second.inspect_ts_end>0)
        {
            interval= vecTmp[i-1].second.inspect_ts_start-vecTmp[i].second.inspect_ts_end-1;
            if(interval >= para->m_InvalidateShowThresholdSec)
            {
                suspicious_show ss;
                ss.adback = vecTmp[i-1].second.ptrTemplet->ad_fileName;
                ss.adprev = vecTmp[i].second.ptrTemplet->ad_fileName;
                ss.start = vecTmp[i].second.inspect_ts_end + 1;
                ss.end = vecTmp[i-1].second.inspect_ts_start -1;
                vecss.push_back(ss);
            }
        }
    }


    return nRet;
}


/*******************************************************************************
* 函数名称：	CalcAvgMatchWeight
* 功能描述：	计算匹配权重的均值
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
float CompareEngine::CalcAvgMatchWeight( std::vector<MatchItem> vecMatch,float &maxWeight)
{
    int len = vecMatch.size();
    float sum = 0;
    float max = 0;
    for(int i = 0 ;i < len ;i++)
    {
        sum += vecMatch[i].fWeight;
        if(max < vecMatch[i].fWeight)
        {
            max = vecMatch[i].fWeight;
        }
    }
    maxWeight = max;
    return sum/len;
}

/*******************************************************************************
* 函数名称：	InsertMatch_DB
* 功能描述：	保存匹配记录
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
bool CompareEngine::InsertMatch_DB()
{

    CppMySQL3DB MatchDB;

    C_Time curtm;
    std::string strcur;
    curtm.setCurTime();
    curtm.getTimeStr(strcur);

    if(MatchDB.open(m_strDB_IP.c_str(),m_strDB_User.c_str(),
            m_strPasswd.c_str(),"AdInspect",m_nPort) == -1)
    {
            printf(0,"mysql open failed!\n");
            return -1;
    }

    std::map<std::string,TempletMatch>::iterator it = m_mapTempletMatch.begin();
    for(;it != m_mapTempletMatch.end();it++)
    {
        char sql[1024]={'\0'};
        TempletMatch&tm = it->second;
        if(tm.vecMatch.size()==0)
        {
            continue;
        }

        int num = tm.vecMatch.size();
        for(int i = 0;i < num ;i++)
        {
            MatchItem &mi = tm.vecMatch[i];

            C_Time tstm;
            std::string strtm;
            tstm.setTimeInt(mi.nInspectTimeStamp);
            tstm.getTimeStr(strtm);

             snprintf(sql,1024,"insert into matchitem"
                               "(task_id,templet_uuid,inspect_order,inspect_ts,"
                               "inspect_time,templet_order,templet_name,weight,time,"
                               "inspect_featrue,templet_featrue,match_count) "
                               "values(\"%s\",\"%s\",%d,%u,"
                               "\"%s\",%d,\"%s\",%.3f,\'%s\',"
                               "%d,%d,%d)",
                                m_curtaskid.c_str(),tm.ptrTemplet->uuid,mi.nInspectIndex,mi.nInspectTimeStamp,
                      strtm.c_str(),mi.nTempletIndex,tm.ptrTemplet->ad_fileName,mi.fWeight,strcur.c_str(),
                      mi.nInspectFeatrue,mi.nTempletFeatrue,mi.nMatchCount);
             int nResult = MatchDB.execSQL(sql);
             if(nResult != -1)
             {
                 loginfo("Save match result successful!(%s)",sql);

             }
             else
             {
                 loginfo("Save match result failed!(%s)",sql);

             }
        }
    }
    return true;
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

    CppMySQL3DB CompareResultDB;

    if(CompareResultDB.open(m_strDB_IP.c_str(),m_strDB_User.c_str(),
                            m_strPasswd.c_str(),"oristarmr",m_nPort) == -1)
    {
            printf(0,"mysql open failed!\n");
            return -1;
    }

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
        snprintf(buf,128,"%s-%s",m_curtaskid.c_str(),tm.ptrTemplet->uuid);
        std::string  id = buf;

        C_Time curtm;
        curtm.setCurTime();
        std::string strCurtime;
        curtm.getTimeStr(strCurtime);

        struct timeval tv;
        gettimeofday(&tv, NULL);
        unsigned long msec =  tv.tv_sec*1000+tv.tv_usec/1000;

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
        vediofullpath =fixbasepath + tm.vediofilename + ".mp4";
        std::string compareimgpath = fixbasepath;

        C_Time starttm,endtm;
        starttm.setTimeInt(tm.inspect_ts_start);
        endtm.setTimeInt(tm.inspect_ts_end);
        std::string strStart;
        std::string strEnd;
        starttm.getTimeStr(strStart);
        endtm.getTimeStr(strEnd);

        int normal_order = tm.ptrTemplet->ad_order;
        int order_status = 0;
        if(normal_order != 0 && normal_order==tm.showorder)
        {
           // 位序正常
           order_status = 1;
        }
        else if(normal_order != 0 && normal_order!=tm.showorder)
        {
            // 位序异常
           order_status = 2;
        }
        else if(normal_order == 0)
        {
            // 位序正常
            order_status = 1;
        }
        else
        {
            // 位序异常
            order_status = 2;
        }

         snprintf(sql,1024,"insert into "
                          "app_monitor(id,advert_id,create_time,sortIdx,advert_back,"
                           "advert_previous,cinema_city,cinema_name,videopath,compare_imgpath,hall_no,"
                           "inspect_end_time,inspect_order,inspect_start_time,monitor_status,imglen) "
                           "values(\"%s\",\"%s\",\"%s\",%u,\"%s\","
                           "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,"
                           "\"%s\",%d,\"%s\",%d,%d)",
                  id.c_str(),tm.ptrTemplet->uuid,strCurtime.c_str(),msec,tm.backAd.c_str(),
                  tm.preAd.c_str(),m_City.c_str(),m_CinemaName.c_str(),vediofullpath.c_str(),compareimgpath.c_str(),m_hallid,
                  strEnd.c_str(),tm.showorder,strStart.c_str(),order_status,tm.inspect_index_end-tm.inspect_index_start+1
                  );
         int nResult = CompareResultDB.execSQL(sql);
         if(nResult != -1)
         {
             loginfo("Save compare result successful!(%s)",sql);

         }
         else
         {
             loginfo("Save compare result failed!(%s)",sql);

         }

    }
    return true;
}

/*******************************************************************************
* 函数名称：	InsertSuspiciousShow_DB
* 功能描述：	可疑播放入库
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
bool CompareEngine::InsertSuspiciousShow_DB(std::vector<suspicious_show> &vecss)
{

    CppMySQL3DB SuspiciousResultDB;

    if(SuspiciousResultDB.open(m_strDB_IP.c_str(),m_strDB_User.c_str(),
                            m_strPasswd.c_str(),"AdInspect",m_nPort) == -1)
    {
            printf(0,"mysql open failed!\n");
            return false;
    }

    for(int i = 0 ;i<vecss.size();i++)
    {
        char sql[1024]={'\0'};
        suspicious_show &ss = vecss[i];

        C_Time starttm,endtm;
        starttm.setTimeInt(ss.start);
        endtm.setTimeInt(ss.end);
        std::string strStart;
        std::string strEnd;
        starttm.getTimeStr(strStart);
        endtm.getTimeStr(strEnd);

        snprintf(sql,1024,"insert into suspicious_show(task_id,adback,adprev,start,end) "
                          "values(\"%s\",\"%s\",\"%s\",\'%s\',\'%s\')",
                 m_curtaskid.c_str(),ss.adback.c_str(),ss.adprev.c_str(),
                 strStart.c_str(),strEnd.c_str());
        int nResult = SuspiciousResultDB.execSQL(sql);
        if(nResult != -1)
        {
            loginfo("Save suspicious show successful!(%s)",sql);

        }
        else
        {
            loginfo("Save suspicious show failed!(%s)",sql);

        }

    }

    return true;
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
    int index = 0;
    for(int i = tm.inspect_index_start ;i<=tm.inspect_index_end;i++)
    {
        index++;
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
            snprintf(name,256,"%s-%d.jpg",tm.ptrTemplet->uuid,index);

            std::string savepath = tm.resultpath;
            if(savepath.rfind("/")!=savepath.size()-1)
            {
                savepath+="/";
            }

            std::string SavePath = savepath + std::string(name);
            CFileEx::CreateFolderForFile(SavePath.c_str());
            Img.SaveJpeg(SavePath);

        }
    }
}

