/*******************************************************************************
* 版权所有 (C) 2010
* 
* 文件名称：	TBuff.h
* 文件标识： 
* 内容摘要：	缓冲区操作辅助类。
* 其它说明：	1、本类所能接受的数据类型仅为基本数据类型；
*				2、本类主要用于需要使用不定长缓冲区的地方；
*				3、本类会自动进行内存的智能分配，策略与vector相同；
*				4、本类的内存数据是连续存放的，可以当内存指针使用；
*				5、本类的函数，如无特殊说明则与C++标准库的函数用法相同；
* 当前版本：	V2.0
* 作    者：	周锋
* 完成日期：	2010-02-04
*******************************************************************************/
#ifndef _TBUFFER_H_12345781234545
#define _TBUFFER_H_12345781234545

namespace Tool
{
	template<class T>
	class TBuff
	{
		//最小分配元素空间个数
		static const size_t MIN_RESERVED = 256;

	public:

		//默认构造函数(默认分配256个字节空间)
		TBuff() 
		{
			m_nReserved = MIN_RESERVED;
			m_nSize = 0;
			m_pData = new T[m_nReserved];
		}

		//带预分配内存大小的构造函数
		TBuff(size_t nReserved)
		{
			if (nReserved < MIN_RESERVED)
			{
				nReserved = MIN_RESERVED;
			}
			m_nReserved = nReserved;
			m_nSize = 0;
			m_pData = new T[m_nReserved];
		}

		//析构函数
		~TBuff()
		{
			delete []m_pData;
		}

		//返回当前的元素数量
		size_t size() const 
		{
			return m_nSize;
		}

		//访问指定位置的元素(注意：本函数不进行参数越界检查)
		T & operator[](size_t nPos) 
		{
			return m_pData[nPos];
		}

		//访问指定位置的元素(注意：本函数不进行参数越界检查)
		const T & operator[](size_t nPos) const 
		{
			return m_pData[nPos];
		}

		//将元素数量修改为nSize，如果元素数量增加，则为多出的元素新分配空间
		void resize(size_t nSize)
		{
			size_t nReserved = m_nReserved;
			while (nReserved < nSize)
			{
				nReserved *= 2;
			}
			if (nReserved != m_nReserved)
			{
				m_nReserved = nReserved;
				T *pNewData = new T[m_nReserved];
				memcpy(pNewData, m_pData, m_nSize * sizeof(T));
				delete []m_pData;
				m_pData = pNewData;
			}
			m_nSize = nSize;
		}

		//向缓冲区追加指定长度的数据(返回追加之后的数据总长度)
		size_t append(const T *pData, size_t nLen)
		{
			size_t nSize = m_nSize;
			resize(m_nSize + nLen);
			memcpy(m_pData + nSize, pData, nLen * sizeof(T));
			return m_nSize;
		}

		//向缓冲区追加一个数据
		size_t append(T t)
		{
			return append(&t, sizeof(t));
		}

		//向缓冲区追加一个TBuff的数据
		size_t append(const TBuff &buff)
		{
			return append(&buff[0], buff.size());
		}

		//数据交换函数
		void swap(TBuff &buff)
		{
			T *pTmp = buff.m_pData;
			size_t tmp1 = buff.m_nSize;
			size_t tmp2 = buff.m_nReserved;

			buff.m_pData = m_pData;
			buff.m_nSize = m_nSize;
			buff.m_nReserved = m_nReserved;

			m_pData = pTmp;
			m_nSize = tmp1;
			m_nReserved = tmp2;
		}

		//清空数据成员
		void clear()
		{
			m_nSize = 0;
		}

		//删除指定的一段元素
		void erase(size_t nPos, size_t nSize)
		{
			if (nPos >= m_nSize)
			{
				return;
			}
			if (nPos + nSize > m_nSize)
			{
				nSize = m_nSize - nPos;
			}
			memmove(m_pData + nPos, m_pData + nPos + nSize, (m_nSize - nSize - nPos) * sizeof(T));
			m_nSize -= nSize;
		}

	private:
		T *m_pData;			//缓冲区净荷指针
		size_t m_nReserved;	//缓冲区内存长度
		size_t m_nSize;		//缓冲区数据长度
	};

}

#endif
