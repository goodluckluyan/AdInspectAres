﻿#ifndef COMPAREENGINE_H
#define COMPAREENGINE_H
#include <tr1/memory>
#include <list>
#include <vector>
#include <map>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "TempletManager/TempletModule.h"
#include "MarkJob/FrameBufferLoop.h"
#include "para/C_Para.h"
#include "utility/osapi/OSThread.h"
#include "TempletManager/VideoCompareModule.h"
#include "database/CppMySQL3DB.h"


typedef struct _MatchItem
{
    int nInspectIndex;
    int nInspectTimeStamp;
    int nTempletIndex;
    float fWeight;
}MatchItem;

typedef std::tr1::shared_ptr<_TEMPLET_ITEM> ptrTempletType;
typedef struct _TempletMatch
{
    _TempletMatch()
    {
        ptrTemplet = NULL;
        inspect_ts_start = 0;
        inspect_ts_end = 0;
        inspect_index_start = 0;
        inspect_index_end  = 0;
        showorder = 0;
    }

    _TEMPLET_ITEM* ptrTemplet;      //模板
    std::vector<MatchItem> vecMatch;//匹配项
    int inspect_ts_start;           //录播时间戳开始
    int inspect_ts_end;             //录播时间戳结束
    int inspect_index_start;        //录播缓冲帧开始相对位置
    int inspect_index_end;          //录播缓冲帧结束相对位置
    int showorder;                  //播放位序
    std::string resultpath;         //比对结果路径，存放匹配图片和视频
    std::string vediofilename;      //视频文件名称
}TempletMatch;

enum Range{ALL,SUBAREA};
typedef struct _SearchArea
{
   _SearchArea()
   {
       area = ALL;
       ptrAVFrame = NULL;
       bufsize = 0;
       featrueNum = 0;
       ptrfeature = NULL;

   }

   ~_SearchArea()
   {
       if(ptrfeature!=NULL)
       {
           delete[] ptrfeature;
       }
   }
   Range area;
   AV_FRAME_INFO * ptrAVFrame;
   unsigned char *ptrfeature;
   unsigned int featrueNum;
   unsigned int bufsize;
   std::list<_TEMPLET_ITEM*> m_lsSearchTempletPtr;
}SearchArea;

typedef std::tr1::shared_ptr<SearchArea> ptrSearchArea;
typedef int (*BC_CompareDone)(void *ptr,std::string &taskid,std::map<std::string,TempletMatch> &templetResult) ;


typedef struct _SpaceRelation
{
    int space;
    int  spaceCount;
    float ratio;
}SpaceRelation;

typedef struct _Relation
{
    int luzhiPo;
    int mobanPo;
    int space;
    float ratio;
    int luzhi_ts;
}Relation;

typedef struct _Location
{
    std::vector<Relation>spaces;
    std::vector<SpaceRelation>demo;

}Location;



class CompareEngine :public OS_Thread
{
public:
    CompareEngine(int hallid,TempletManager *ptrTempletMgr);
    ~CompareEngine();

    // 设置比对完成回调函数
    static void SetBCFun(void * ptr,BC_CompareDone ptrFun);

    // 比对区域，数据库参数，
    int Init(std::string &dbip,std::string &dbuser,std::string &passwd,std::string dbname,
                 int port,C_Para::Rect &rect);

    // 比较
    int Compare(std::string &taskid,FrameBufferLoop *);

    // 线程函数
    int Routine();
private:

    // 创建每个录播图的搜索模板的范围
    int CreateSearchArea(FrameBufferLoop *fbl);

    // 对录播图按搜索范围进行比对
    int Image_compare(int index,ptrSearchArea inspect,bool bSpeedPriority=false);

    // 对录播图的邻居进行标注
    int Label_inspect(int start,int end,_TEMPLET_ITEM* lable);

    // 按模板统计结果并定位录播位置
    int SummaryResultsAndLocatorPo();

    // 保存比对成功的图片
    bool SaveInspectImage(TempletMatch &tm);

    // 结果入库
    bool InsertResult_DB();

private:
    // 厅号
    int m_hallid;

    // 录播数据和搜索结构
    std::map<int,ptrSearchArea> m_mapInspectSearchArea;

    // 模板数据和匹配对
    std::map<std::string,TempletMatch> m_mapTempletMatch;

    // 模板管理模块指针
    TempletManager *m_ptrTempletMgr;

    // 模板数据
    TEMPLET_LIST m_rawTemplet;

    // 比对完成回调
    static void * m_ptrUser;
    static BC_CompareDone m_ptrCompareDoneFun;

    // 特征提取和比对模块
    VideoCompareModule m_featrueExtract;

    // 匹配成功阈值
    float m_threshold ;

    // 数据库操作
    CppMySQL3DB m_CompareResultDB;

    // 录播比对区域
    C_Para::Rect m_rect;

    // 当前任务id
    std::string m_curtaskid;

    void * m_loopbuffer;

};

class ImgBuf
{
public:

    typedef struct BITMAPFILEHEADER
    {
        unsigned short bfType;
        unsigned int  bfSize;
        unsigned short bfReserved1;
        unsigned short bfReserved2;
        unsigned int bfOffBits;
    }__attribute__((packed)) BITMAPFILEHEADER;

    typedef struct BITMAPINFOHEADER
    {
        u_int32_t biSize;
        u_int32_t biWidth;
        u_int32_t biHeight;
        u_int16_t biPlanes;
        u_int16_t biBitCount;
        u_int32_t biCompression;
        u_int32_t biSizeImage;
        u_int32_t biXPelsPerMeter;
        u_int32_t biYPelsPerMeter;
        u_int32_t biClrUsed;
        u_int32_t biClrImportant;
    }__attribute__((packet)) BITMAPINFODEADER;

    ImgBuf(unsigned char * pYUV,long size,int width,int height)
    {
        m_pBGR24 =(char *) malloc(width*height*3);
        cv::Mat dst(height,width,CV_8UC3,m_pBGR24);
        cv::Mat src(height + height/2,width,CV_8UC1,pYUV);
        cvtColor(src,dst,::CV_YUV2BGR_YV12);
        m_lsize = size;
        m_nWidth = width;
        m_nHeight = height;
    }

    ~ImgBuf()
    {

        if(m_pBGR24!=NULL)
        {
           free(m_pBGR24);
           m_pBGR24 = NULL;
        }


    }

    void Savebmp(std::string fullpath)
    {

        int lineBytes = m_nWidth*3;
//        char fileName[1024]={0};
//        char * bmpSavePath = "./pic/%s.bmp";

//        if(access("./pic",0)==-1)
//        {
//           mkdir("./pic",0755);
//        }
//        sprintf(fileName,bmpSavePath ,name.c_str());
        FILE *pDestFile = fopen(fullpath.c_str(), "wb");
        if(NULL == pDestFile)
        {
            printf("open file %s failed\n",fullpath.c_str());
            return;
        }
        BITMAPFILEHEADER btfileHeader;
        btfileHeader.bfType = 0x4d42;//mb
        btfileHeader.bfSize = lineBytes*m_nHeight;
        btfileHeader.bfReserved1 = 0;
        btfileHeader.bfReserved2 = 0;
        btfileHeader.bfOffBits = sizeof(BITMAPFILEHEADER);

        BITMAPINFOHEADER bitmapinfoheader;
        bitmapinfoheader.biSize = 40;
        bitmapinfoheader.biWidth = m_nWidth;
        bitmapinfoheader.biHeight = m_nHeight;
        bitmapinfoheader.biPlanes = 1;
        bitmapinfoheader.biBitCount = 24;
        bitmapinfoheader.biCompression = 0;
        bitmapinfoheader.biSizeImage = lineBytes*m_nHeight;
        bitmapinfoheader.biXPelsPerMeter = 0;
        bitmapinfoheader.biYPelsPerMeter = 0;
        bitmapinfoheader.biClrUsed = 0;
        bitmapinfoheader.biClrImportant = 0;

        int i=0;
        fwrite(&btfileHeader, sizeof(BITMAPFILEHEADER), 1, pDestFile);
        fwrite(&bitmapinfoheader, sizeof(BITMAPINFODEADER), 1, pDestFile);
        for(i=m_nHeight-1; i>=0; i--)
        {
           fwrite(m_pBGR24+i*lineBytes, lineBytes, 1, pDestFile);
        }

        fclose(pDestFile);
    }
public:

    char * m_pBGR24;

    unsigned int m_lsize;
    unsigned int m_nWidth;
    unsigned int m_nHeight;
};

#endif // COMPAREENGINE_H
