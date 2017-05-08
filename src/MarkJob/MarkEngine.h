#ifndef MARKENGINE_H
#define MARKENGINE_H

#include <string>
#include <vector>

typedef struct  __rect
{
    __rect()
    {
      left = 0;
      top = 0;
      right = 0;
      bottom = 0;
    }

    __rect(int l,int t,int r,int b)
    {
      left = l;
      top = t;
      right = r;
      bottom = b;
    }

   __rect(__rect &obj)
   {

       left = obj.left;
       top = obj.top;
       right = obj.right;
       bottom = obj.bottom;


   }

    __rect& operator=(__rect &obj)
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
}Rect;

typedef struct __Imgfeatrue
{
   void * featrue;                         // 图片特征集合
   std::string path;                       // 图片路径
   std::string fpath;                      // 图片特征文件路径
}ImgFeatrue;

typedef struct __FeatrueGroup
{
        std::string m_name;                // 模板名称
        std::vector<ImgFeatrue> m_fgroup;  // 模板特征
}FeatrueGroup;

typedef struct __Compare_Para
{
    bool upright;                           // run in rotation invariant mode?
    int octaves ;                           // number of octaves to calculate
    int intervals;                          // number of intervals per octave
    int init_sample ;                       // initial sampling step
    float thres;                            // blob response threshold
}Compare_para;

class MarkEngine
{
public:
    //MarkEngine构造
    //@LongbiaoPath 模板图片路径
    //@CheckRect 监播图像龙标检测区域
    MarkEngine(std::string LongbiaoPath,Rect CheckRect);

    virtual ~MarkEngine();

public:
    //加载龙标特征
    //@return  0：没有找到特征加载失败  >0：加载成功
    int LoadFeatrue();

    //查找监播图像中的龙标
    //@buf监播图缓存指针，解码器解码后一帧的数据yuv12格式
    //@size缓存大小
    //@width 监播图宽度
    //@height监播图长度
    //@id录播图片ID,如果判断为龙标会以ID作为文件名保存图片
    //@return -1:出错  0没有找到  >0找到
    int FindLongbiao(char * buf,int size,int width,int height,std::string id);

private:
    // 检测龙标
    int  CheckLongbiao(void *inspectImg);

    // 获取信息熵
    int  GetEntropy(void *inspectImg);

    // 获取rgb信息熵
    bool GetEntropy_rgb(void *pIpImage,float *r_sum_r,float *g_sum_r,float *b_sum_r);

    // 获取gray信息熵
    float GetEntropy_gray(void *pIpImage);

    // 判断是否和特征匹配
    bool  Ismatch( void * matches);

    // 从指定特征目录提取特征
    int   ExtractFromDir();

private:
    bool m_bLoad;                   // featrue whether load
    std::string m_ModulePath;       // module path
    Rect m_Rect;                    // sensitive region
    FeatrueGroup m_FModule;         // feature module
    float m_threshold;              // show status threshold
    Compare_para m_CompareParameter;// compare parameter
};


#endif
