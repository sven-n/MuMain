

#ifndef _STRINGSET_H_
#define _STRINGSET_H_

#pragma once

#pragma warning(disable : 4786)
#include <string>
#include <map>

namespace leaf {

	template <class T>
	class TStringSet
	{
		typedef std::basic_string<T, std::char_traits<T>, std::allocator<T> > string_type;
		typedef std::map<int, string_type> string_map_type;
		typedef string_map_type& string_map_referece;
		typedef const string_map_type& string_map_const_referece;
		typedef TStringSet<T>&	referece_type;
		typedef const TStringSet<T>& const_referece_type;

		string_map_type	m_mapString;
		const string_type m_strNone;

	public:
		TStringSet() {}
		TStringSet(const TStringSet<T>& _StringSet)
		{
			string_map_const_referece _ref = _StringSet.m_mapString;
			string_map_type::const_iterator mi = _ref.begin();
			for(; mi != _ref.end(); mi++)
				Add((*mi).first, (*mi).second);
		}
		~TStringSet() { RemoveAll(); }
		
		bool Add(int key, const T* szString)
		{
			string_map_type::iterator mi = m_mapString.find(key);
			if(mi == m_mapString.end())
			{
				m_mapString.insert( string_map_type::value_type(key, szString) );
				return true;
			}
			return false;
		}
		bool Add(int key, const string_type& strString)
		{
			string_map_type::iterator mi = m_mapString.find(key);
			if(mi == m_mapString.end())
			{
				m_mapString.insert( string_map_type::value_type(key, strString) );
				return true;
			}
			return false;
		}
		bool Remove(int key)
		{
			string_map_type::iterator mi = m_mapString.find(key);
			if(mi != m_mapString.end())
			{
				m_mapString.erase(mi);
				return true;
			}
			return false;
		}
		void RemoveAll() { m_mapString.clear(); }

		size_t GetCount() { return m_mapString.size(); }
		int GetKey(int index)
		{
			string_map_type::const_iterator mi = m_mapString.begin();
			for(int count=0; mi != m_mapString.end(); mi++, count++)
				if(count == index)
					return (*mi).first;
			return -1;
		}
		const string_type& GetObj(int index)
		{
			string_map_type::const_iterator mi = m_mapString.begin();
			for(int count=0; mi != m_mapString.end(); mi++, count++)
				if(count == index)
					return (*mi).second;
			return -1;
		}

		const T* Find(int key)
		{
			string_map_type::const_iterator mi = m_mapString.find(key);
			if(mi != m_mapString.end())
				return ((*mi).second).c_str();			
			return NULL;
		}
		const string_type& FindObj(int key)
		{
			string_map_type::const_iterator mi = m_mapString.find(key);
			if(mi != m_mapString.end())
				return (*mi).second;
			return m_strNone;
		}

		const T* operator [] (int index)
		{ return FindObj(index).c_str(); }

		TStringSet<T>& operator = (const TStringSet<T>& _StringSet)
		{
			string_map_const_referece _ref = _StringSet.m_mapString;
			string_map_type::const_iterator mi = _ref.begin();
			for(; mi != _ref.end(); mi++)
				Add((*mi).first, (*mi).second);
			return *this;
		}
	};

	typedef TStringSet<char>	CStringSet;
	typedef TStringSet<wchar_t>	CStringSetW;
}

#endif /* _STRINGSET_H_ */
