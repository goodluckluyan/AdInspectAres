/*******************************************************************************
* 版权所有 (C) 2012
* 
* 文件名称： TMap.h
* 文件标识： 
* 内容摘要： 使用vector模拟map，为了废弃map的自动排序功能
* 其它说明： 
* 当前版本： V1.0
* 作    者： 白福铭
* 完成日期： 2012-10-26
*******************************************************************************/
#pragma once

#include <vector>

namespace Tool
{
	template<class K, class T>
	class TMap
	{
	public:

		// 映射列表定义
		typedef std::vector<std::pair<K, T> > TVec;

		// 迭代器定义
		typedef typename TVec::iterator iterator;
		typedef typename TVec::const_iterator const_iterator;

		// 构造函数
		TMap() {}

		// 析构函数
		~TMap() {}

		T & operator [] (const K &_Key)
		{
			for (iterator it = m_vecT.begin(); it != m_vecT.end(); ++it)
			{
				if (_Key == it->first)
				{
					return it->second;
				}
			}

			m_vecT.push_back(std::make_pair(_Key, T()));
			return m_vecT.rbegin()->second;
		}

		size_t size() const
		{
			return m_vecT.size();
		}

		void clear()
		{
			m_vecT.clear();
		}

		iterator begin()
		{
			return m_vecT.begin();
		}

		const_iterator begin() const
		{
			return m_vecT.begin();
		}

		iterator end()
		{
			return m_vecT.end();
		}

		const_iterator end() const
		{
			return m_vecT.end();
		}

		iterator find(const K &_Key)
		{
			for (iterator it = m_vecT.begin(); it != m_vecT.end(); ++it)
			{
				if (_Key == it->first)
				{
					return it;
				}
			}

			return m_vecT.end();
		}

		const_iterator find(const K &_Key) const
		{
			for (const_iterator it = m_vecT.begin(); it != m_vecT.end(); ++it)
			{
				if (_Key == it->first)
				{
					return it;
				}
			}

			return m_vecT.end();
		}

		const T & at(size_t _Pos) const
		{
			return m_vecT[_Pos].second;
		}

		T & at(size_t _Pos)
		{
			return m_vecT[_Pos].second;
		}

		iterator erase(iterator _Where)
		{
			return m_vecT.erase(_Where);
		}

		size_t erase(const K &_Key)
		{
			size_t nCount = 0;
			for (const_iterator it = m_vecT.begin(); it != m_vecT.end(); ++it)
			{
				if (_Key == it->first)
				{
					it = m_vecT.erase(it);
					nCount++;
				}
			}

			return nCount;
		}

	protected:

		// 映射列表
		TVec m_vecT;
	};

}
