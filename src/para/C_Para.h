//@file:C_Para.h
//@brief: 包含类C_Para。
//C_Para:读取系统配置文件中的各个参数。
//@author:wangzhongping@oristartech.com
//dade:2012-07-12


#ifndef _TMS20_PARA
#define _TMS20_PARA
#include <string>
#include <pthread.h>
using namespace std;




class C_Para
{
public:
    typedef struct  Rect
    {
        Rect()
        {
          left = 0;
          top = 0;
          right = 0;
          bottom = 0;
        }

        Rect(int l,int t,int r,int b)
        {
          left = l;
          top = t;
          right = r;
          bottom = b;
        }

       Rect(Rect &obj)
       {

           left = obj.left;
           top = obj.top;
           right = obj.right;
           bottom = obj.bottom;


       }

        Rect& operator=(Rect &obj)
        {
            if(this != &obj)
            {
                left = obj.left;
                top = obj.top;
                right = obj.right;
                bottom = obj.bottom;

            }
            return *this;
        }

        int width(){return right-left;};
        int height(){return bottom-top;};
        int left;
        int top;
        int right;
        int bottom;
    };

 //method:
    static C_Para* GetInstance();
	static void DestoryInstance();
    ~C_Para();
    //读取配置参数。
    int ReadPara();

protected:
     C_Para();
public:    
//Property:  

	//日志存放路径
    string m_strLogPath;

	//本机webservice服务打开的端口
	int m_nWebServicePort;

	//写日志级别
	int m_nWirteLogLevel;

    // 映前广告播放时间
    int m_AdShow_sec;

    // 保存的最大帧数量
    int m_Max_frame_count;

    // 保存的视频时间
    int m_VideoFile_SplitSec;

    // 解码帧率：1秒解n帧
    int m_Decode_rate;

    // 龙标模板路径
    std::string m_Mark_path;

    // 检测到龙标后保存图片路径
    std::string m_Mark_Store_path;

    // 视频下载路径
    std::string m_Download_Vedio_path;

    // 广告模板保存路径
    std::string m_Templet_Store_path;

    // 广告模板视频路径
    std::string m_Templet_Video_path;


    // 转码后图片分辨率调整
    int m_Decode_width;
    int m_Decode_height;

    // 彩色/灰度格式设置
    bool m_bIsGray;

    // 数据库信息
    std::string m_DB_IP;
    std::string m_DB_User;
    std::string m_DB_Passwd;
    int m_DB_Port;
    int NVRCtr_DB_Name;


    // 录播比对区域
    Rect m_CompareRect;

    // 匹配保存路径(找到匹配之后保存图片)
    std::string m_match_store_path;

    // 影院放映时间用秒表示
    int m_ShowingJobStartSec;
    int m_ShowingJobEndSec;

    // 应用程序路径
    std::string m_strInipath;

    // 比对成功的阈值
    float m_WeightThreshold;

    // 比对成功的匹配数量阈值
    int m_MatchCountThreshold;

    // 是否速度优先
    bool m_IsSpeedPriority;

    // 多播时间阈值
    int m_InvalidateShowThresholdSec;

    // 比对类型
    std::string m_FeatrueType;

private:
    static C_Para *m_pInstance;
	pthread_rwlock_t m_rwlk_main;
};



#endif //_TMS20_PARA
