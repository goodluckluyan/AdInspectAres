/////////////////////////////////////////////////////////////
/// XLQ:对Sift和Surf算法接口的封装
/////////////////////////////////////////////////////////////
#ifndef _VIDEO_COMPARE_MODULE_H_
#define _VIDEO_COMPARE_MODULE_H_

class VideoCompareModule
{
public:
    VideoCompareModule();
    ~VideoCompareModule();

    /////////////////////////////////////////////////////////////
    /// 初始化视频比对模块的内置算法，目前支持Sift和Surf两种视频比对算法。
    /// videoCompareType，输入，设置内置算法的类型:0表示Sift算法，1表示Surf算法。
    /// 在调用视频比对模块前，如果没有调用该函数，会默认使用Sift算法。
    int InitModule(int videoCompareType);


    /////////////////////////////////////////////////////////////
    /// 计算一段RGB像素值缓存的特征值，不需要将特征值保存在本地文件中。
    /// rgbBufferData，输入，要进行特征值计算的RGB像素值缓存的首地址，
    /// rgbBufferLength，输入，要进行特征值计算的RGB像素值缓存的长度，
    /// widthSize，输入，要进行特征值计算的RGB像素值缓存的图片宽度，
    /// heightSize，输入，要进行特征值计算的RGB像素值缓存的图片高度，
    /// featureNumber，输出，本段RGB像素值缓存的特征值数。
    int ExportFeature(const char *rgbBufferData,
        unsigned int &rgbBufferLength,
        unsigned int &widthSize,
        unsigned int &heightSize,
        int &featureNumber);


    /////////////////////////////////////////////////////////////
    /// 计算一张BMP格式图片的特征值，并将特征值保存在本地文件中。
    /// bmpFileName，输入，要进行特征值计算的BMP格式图片文件名，
    /// 注意:文件名中应包含文件路径。如"/mnt/bmp/000001.bmp"。
    /// featureFileName，输入，要保存特征值数据的本地文件名，
    /// 注意:文件名中应包含文件路径。如"/mnt/feature/000001.feature"
    /// featureNumber，输出，本幅图片的特征值数。
    int ExportFeature(const char *bmpFileName, 
        const char *featureFileName,
        int &featureNumber);


    /////////////////////////////////////////////////////////////
    /// 在计算一张图片的特征值后，可以从内置的缓存中获取特征值数据。
    /// 在获取下一张图片的特征值后，缓存中原有的特征值数据将被新的特征值数据覆盖。
    /// featureBufferData，输出，存储了特征值数据的缓存首地址。
    /// featureBufferLength，输出，存储了特征值数据的缓存长度。
    int GetFeatureBuffer(char *featureBufferData,
        unsigned int &featureBufferLength);


    /////////////////////////////////////////////////////////////
    /// 从保存了特征值数据的本地文件中，读取特征值数据，并将特征值数据保存在一块缓存中。
    /// featureFileName，输入，保存了特征值数据的本地文件名，
    /// 注意:文件名中应包含文件路径。如"/mnt/feature/000001.feature"
    /// featureNumber，输入，特征值数据中的特征值数量。
    /// featureBufferData，输出，存储了特征值数据的缓存首地址。
    /// 注意:存储特征值数据的缓存需要在调用该函数前预先建立好，
    /// 建立时特征值数据的缓存长度不是固定的，其大小是由对应图片中边缘数量的多少决定的，
    /// 因此应根据对应图片的实际特征值数来建立缓存。一般1个特征值需要占用不大于1024字节的空间。
    /// featureBufferLength，输出，存储了特征值数据的缓存长度。
    int ImportFeature(const char *featureFileName,
        int &featureNumber,
        char *featureBufferData,
        unsigned int &featureBufferLengh);


    /////////////////////////////////////////////////////////////
    /// 对两块保存了特征值数据的缓存进行视频比对，从而获得匹配的特征值数量。
    /// featureBufferData1，输入，保存了第一块特征值数据的缓存首地址，
    /// featureBufferLength1，输入，保存了第一块特征值数据的缓存长度，
    /// widthSize1，输入，第一块特征值数据对应的图像长度，
    /// heightSize1，输入，第一块特征值数据对应的图像宽度，
    /// featureNumber1，输入，第一块特征值数据的特征值数量。
    /// featureBufferData2，输入，保存了第二块特征值数据的缓存首地址，
    /// featureBufferLength2，输入，保存了第二块特征值数据的缓存长度，
    /// widthSize2，输入，第二块特征值数据对应的图像长度，
    /// heightSize2，输入，第二块特征值数据对应的图像宽度，
    /// featureNumber2，输入，第二块特征值数据的特征值数量。
    /// matchNumber，输出，返回的两块特征值数据的匹配特征值数量。
    /// 【说明】:上层程序通过判断匹配特征值的数量与原始图片特征值数量的百分比
    /// （比如是否大于等于10%），来判断这两张图片是不是同一张。
    int CompareFeature(const char *featureBufferData1, 
        unsigned int &featureBufferLength1,
        unsigned int &widthSize1,
        unsigned int &heightSize1,
        int &featureNumber1,
        const char *featureBufferData2, 
        unsigned int &featureBufferLength2,
        unsigned int &widthSize2,
        unsigned int &heightSize2,
        int &featureNumber2,
        int &matchNumber);


    /////////////////////////////////////////////////////////////
    /// 对两个图片文件进行视频比对，从而获得匹配的特征值数量。
    /// 【说明】:通过本函数可以输入两张要进行视频比对的图片，
    /// 然后将比对的结果保存在同一张大图中，这样可以直观的查看视频比对效果。
    /// 本函数主要用于测试，在实际上线产品中不会使用。
    /// pictureFileName1，输入，第一个图片文件的文件名（包含路径），
    /// pictureFileName2，输入，第二个图片文件的文件名（包含路径），
    /// stackFileName1，输出，将前两张图片的视频比对结果保存在同一个文件中（包含路径），
    /// stackFileName2，输出，将前两张图片互换后，再进行视频比对，然后将结果保存在同一个文件中（包含路径），
    /// matchNumber，输出，返回两张图片文件的匹配特征值数量。
    int CompareFeature(const char *pictureFileName1, 
        const char *pictureFileName2, 
        const char *stackFileName1,
        const char *stackFileName2,
        int &matchNumber);


    /////////////////////////////////////////////////////////////
    /// XLQ:获得返回信息。
    /////////////////////////////////////////////////////////////
    const char *GetResultString();

protected:

private:
    /// XLQ:当前的特征值缓存及长度。
    char *currentFeatureBufferData;
    unsigned int currentFeatureBufferLength;

    /// XLQ:当前使用的内置算法类型，0为Sift算法，1为Surf算法。
    int currentVideoCompareType;

    //unsigned int currentTime;

    /// XLQ:设置返回信息。
    void SetResultString(const char *resultString);

    ///// XLQ:设置包含了文件名称的返回信息。
    //void SetResultString(const char *fileName,
    //    const char *resultString);

    /// XLQ:返回的信息。
    const char *resultBuffer;

    ///// XLQ:组成返回信息用的缓冲区。
    //char *resultTemp;
};

#endif
