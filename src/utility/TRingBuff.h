#ifndef _TRINGBUFF_H_
#define _TRINGBUFF_H_

#include <string>

#ifndef ASSERT
#define ASSERT 
#endif

//如果需要禁用日志，使用前请定义宏: #define HIDE_RING_BUFF_LOG

namespace Tool
{
	template<class T>
	class TRingBuff 
	{
		//数据
		T *m_pData;

		//缓存中的数据帧数
		size_t m_nCount;

		//当前起始位置
		size_t m_nStart;

		//最大缓存的数据帧数目
		size_t m_nMaxSize;

		//模块名称
		std::string m_strName;

	public:

		//构造(lpszName为缓冲区名称，用于日志输出时鉴别是哪个缓冲区的日志)
		TRingBuff(size_t nMaxSize, const char *lpszName = "default") 
			: m_nCount(0), m_nStart(0), m_nMaxSize(nMaxSize), m_strName(lpszName)
		{
			m_pData = new T[m_nMaxSize];
		}

		//析构
		~TRingBuff(){delete []m_pData;}

		//设置缓冲区的名称
		void SetName(const char *lpszName)
		{
			m_strName = std::string(lpszName);
		}

		//缓冲区是否为空
		bool empty() const {return m_nCount == 0;}

		//缓冲区元素个数
		size_t size() const {return m_nCount;}

		//缓冲区容量大小
		size_t max_size() const {return m_nMaxSize;}

		//清空缓冲区
		void clear(){m_nCount = 0;}

		//删除缓冲区第一个元素
		bool pop_front()
		{
			if (empty())
			{
				return false;
			}
			m_nStart++; 
			if (m_nMaxSize == m_nStart)
			{
				m_nStart = 0;
			}
			m_nCount--;
			return true;
		}

		//删除缓冲区最后一个元素
		bool pop_pack()
		{
			if (empty())
			{
				return false;
			}
			m_nCount--;
			return true;
		}

		//向缓冲区末尾追加一个数据
		bool push_back()
		{
			if (m_nMaxSize == m_nCount)
			{
#ifndef HIDE_RING_BUFF_LOG
#ifdef LogN
				LogN(5001)("---<%s>缓存数据帧超过%d帧，丢弃第一帧。", m_strName.c_str(), m_nMaxSize);
#elif defined TRACE
				TRACE("---<%s>缓存数据帧超过%d帧，丢弃第一帧。\n", m_strName.c_str(), m_nMaxSize);
#else
				printf("---<%s>缓存数据帧超过%d帧，丢弃第一帧。\n", m_strName.c_str(), m_nMaxSize);
#endif
#endif
				m_nStart++;
				if (m_nMaxSize == m_nStart)
				{
					m_nStart = 0;
				}
			}
			else
			{
				m_nCount++;
			}

			T &node = back();
			return true;
		}

		//取出第一个数据
		T & front()
		{
			return (*this)[0];
		}

		//取出最后一个数据
		T & back()
		{
			return (*this)[m_nCount - 1];
		}

		//取出指定位置的数据
		T & operator[](size_t nPos)
		{
			ASSERT(m_nCount > nPos);
			size_t nRealPos = m_nStart + nPos;
			if (nRealPos >= m_nMaxSize)
			{
				nRealPos -= m_nMaxSize;
			}
			return m_pData[nRealPos];
		}

	};

}

#endif

