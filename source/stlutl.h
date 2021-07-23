

#ifndef _STLUTL_H_
#define _STLUTL_H_

#pragma once

#include <ctype.h>

#pragma warning(disable : 4786)
#include <string>
#include <list>
#include <vector>
#include <queue>
#include <deque>
#include <stack>

namespace leaf {
	
	inline std::string make_lower(const std::string& str) {
		std::string lower;
		for(int i=0; i<str.size(); i++) {
			if(isupper(str[i])) 
				lower += (char)tolower(str[i]);
			else
				lower += str[i];
		}
		return lower;
	}
	inline std::string make_upper(const std::string& str) {
		std::string upper;
		for(int i=0; i<str.size(); i++) {
			if(islower(str[i])) 
				upper += (char)toupper(str[i]);
			else
				upper += str[i];
		}
		return upper;
	}

	
	// pre-declaration
	template<class T> class tree;
	
	template<class _Td,class _Ts>
	class mem_var_t : public std::unary_function<_Ts, _Td> {
	public:
		explicit mem_var_t(_Td _Ts::*m)
			:m_ptr(m) {}	
		_Td operator () (const _Ts& x) const {
			return x.*m_ptr;	
		}
		_Td operator () (const _Ts* x) const {
			return x->*m_ptr;
		}
	private:
		_Td _Ts:: *m_ptr;
	};
	
	
	template<class _Td,class _Ts> 
	inline mem_var_t<_Td,_Ts> mem_var(_Td _Ts::* m) {
		return mem_var_t<_Td,_Ts>(m);
	}/*	Usage

		struct test{ int a,b,c; };
		test t = { 10, 100, 1000 };
		int cs = mem_var(&test::c)(t);  
		int cp = mem_var(&test::c)(&t);
	*/


	// find with member

	template<class T,class _Td,class _Ts>	//. for list
	inline T* find_with_member(const std::list<T>& container, _Td _Ts::* m, const _Td& value) {
		std::list<T>::const_iterator iter = container.begin();
		for(; iter != container.end(); iter++)
			if(mem_var(m)(*iter) == value)
				return (T*)(&(*iter));
		return NULL;
	}
	
	template<class T,class _Td,class _Ts>	//. for vector
	inline T* find_with_member(const std::vector<T>& container, _Td _Ts::* m, const _Td& value) {
		std::vector<T>::const_iterator iter = container.begin();
		for(; iter != container.end(); iter++)
			if(mem_var(m)(*iter) == value)
				return (T*)(&(*iter));
		return NULL;
	}

	template<class T,class _Td,class _Ts>	//. for tree
	inline T* find_with_member(const leaf::tree<T>& container, _Td _Ts::* m, const _Td& value) {
		T* _ptrT = NULL;
		leaf::tree<T>::const_iterator iter = container.begin();
		for(; iter != container.end(); iter++) {
			if(mem_var(m)((*iter).value()) == value)
				return (T*)(&(*iter).value());
			if(_ptrT = find_with_member((*iter), m, value))
				break;
		}
		return _ptrT;
	}

	// delete all

	template<class T>	//. for list
	inline void delete_all(std::list<T>& container) {
		std::list<T>::iterator iter = container.begin();
		for(; iter != container.end(); iter++)
			delete (*iter);
		container.clear();
	}

	template<class T>	//. for vector
	inline void delete_all(std::vector<T>& container) {
		std::vector<T>::iterator iter = container.begin();
		for(; iter != container.end(); iter++)
			delete (*iter);
		container.clear();
	}

	template<class T>	//. for queue
	inline void delete_all(std::queue<T>& container) {
		while(!container.empty()) {
			delete container.front();
			container.pop();
		}
	}

	template<class T>	//. for priority_queue
	inline void delete_all(std::priority_queue<T>& container) {
		while(!container.empty()) {
			delete container.top();
			container.pop();
		}
	}

	template<class T>	//. for deque
	inline void delete_all(std::deque<T>& container) {
		while(!container.empty()) {
			delete container.front();
			container.pop();
		}
	}

	template<class T>	//. for stack
	inline void delete_all(std::stack<T>& container) {
		while(!container.empty()) {
			delete container.top();
			container.pop();
		}
	}
	
	template<class T>	//. for tree
	inline void delete_all(leaf::tree<T>& container, bool self_release = true) {
		if(self_release && container.value())
			delete container.value();	//. self-release
		leaf::tree<T>::iterator iter = container.begin();
		for(; iter != container.end(); iter++)
			delete_all((*iter), true);	//. delete inferior tree
		container.clear();
	}
	
	// pop all
	template<class T>	//. for stl containers - queue, deque, priority_queue, stack
	inline void pop_all(T& container)
	{
		while(!container.empty())
			container.pop();
	}

	// tree
	template<class T>
	class tree {
	public:
		typedef tree<T>	tree_type;
#ifdef _VS2008PORTING
		typedef typename std::allocator<tree_type> allocator;
		typedef typename allocator::difference_type difference_type;
		typedef typename allocator::value_type value_type;
		typedef typename allocator::pointer pointer;		// tree<T>*
		typedef typename allocator::const_pointer const_pointer;	// const tree<T>*
		typedef typename allocator::reference reference;	// tree<T>&
		typedef typename allocator::const_reference const_reference;	// const tree<T>&

		typedef typename std::deque<tree_type>::iterator iterator;
		typedef typename std::deque<tree_type>::const_iterator const_iterator;
		typedef typename std::deque<tree_type>::reverse_iterator reverse_iterator;
		typedef typename std::deque<tree_type>::const_reverse_iterator const_reverse_iterator;
#else // _VS2008PORTING
		typedef std::allocator<tree_type> allocator;
		typedef allocator::difference_type difference_type;
		typedef allocator::value_type value_type;
		typedef allocator::pointer pointer;		// tree<T>*
		typedef allocator::const_pointer const_pointer;	// const tree<T>*
		typedef allocator::reference reference;	// tree<T>&
		typedef allocator::const_reference const_reference;	// const tree<T>&

		typedef std::deque<tree_type>::iterator iterator;
		typedef std::deque<tree_type>::const_iterator const_iterator;
		typedef std::reverse_iterator<iterator, value_type, reference, pointer, difference_type>
			reverse_iterator;
		typedef std::reverse_iterator<const_iterator, value_type, const_reference, const_pointer, difference_type>
			const_reverse_iterator;
#endif // _VS2008PORTING
		
		tree() : m_parent_ptr(NULL) { ZeroMemory(&m_value, sizeof(T)); }
		tree(const T& _value) : m_parent_ptr(NULL) { m_value = _value; }
		tree(const tree<T>& _tree)		//. 중간노드를 루트로 복사할때는 copy를 사용할것!
		{
			m_value = _tree.m_value;
			m_parent_ptr = _tree.m_parent_ptr;		//. *
			const_iterator iter = _tree.begin();
			for(; iter != _tree.end(); iter++)
				push_back((*iter));
		}
		~tree() { m_list.clear(); }

		iterator begin() { return m_list.begin(); }
		const_iterator begin() const { return (const_iterator)m_list.begin(); }
		iterator end() { return m_list.end(); }
		const_iterator end() const { return (const_iterator)m_list.end(); }
		reverse_iterator rbegin() { return m_list.rbegin(); }
		const_reverse_iterator rbegin() const { return (const_reverse_iterator)m_list.rbegin(); }
		reverse_iterator rend() { return m_list.rend(); }
		const_reverse_iterator rend() const { return (const_reverse_iterator)m_list.rend(); }
		reference front() { return m_list.front(); }
		const_reference front() const { return (const_reference)m_list.front(); }
		reference back() { return m_list.back(); }
		const_reference back() const { return (const_reference)m_list.back(); }

		pointer parent() { return m_parent_ptr; }
		const_pointer parent() const { return (const_pointer)(m_parent_ptr); }

		void push_back(const tree<T>& subtree)
		{	
			m_list.push_back(subtree);	//. to be copied
			back().m_parent_ptr = this;
		}
		void push_front(const tree<T>& subtree) 
		{
			m_list.push_front(subtree);	//. to be copied
			front().m_parent_ptr = this;
		}
		void pop_back() { m_list.pop_back(); }
		void pop_front() { m_list.pop_front(); }
		iterator insert(iterator iter, const tree<T>& subtree) 
		{	
			iterator _new_iter = m_list.insert(iter, subtree);
			(*_new_iter).m_parent_ptr = this;
			
			return _new_iter;
		}
		iterator erase(iterator iter) { return m_list.erase(iter); }
		void clear() { m_list.clear(); }

		T& value() { return m_value; }
		const T& value() const{ return (const T&)(m_value); }

		bool is_root() const { return (bool)(m_parent_ptr == NULL); }
		bool is_leaf() const { return (bool)(m_parent_ptr != NULL); }
		bool is_descendant_of(const tree<T>& ancestor) const
		{
			if(m_parent_ptr) {
				if(m_parent_ptr == &ancestor)
					return true;
				return m_parent_ptr->is_descendant_of(ancestor);
			}
			return false;
		}
		bool empty() const { return m_list.empty(); }
		size_t size() const { return m_list.size(); }	//. 자식들의 수
		size_t max_size() const { return m_allocator.max_size(); }
		size_t degree() const	//. 자손들의 수
		{
			size_t degree = 0;
			for(const_iterator iter = begin(); iter != end(); iter++, degree++)
				degree += (*iter).degree();
			return degree;
		}
		size_t level() const // 트리 안에서의 깊이
		{
			if(m_parent_ptr == NULL)
				return 0;
			return m_parent_ptr->level()+1;
		}

		reference at(size_t n)
		{ 
			if(size() <= n) throw; 
			return (*(m_list.begin() + n)); 
		}
		const_reference at(size_t n) const 
		{ 
			if(size() <= n) throw; 
			return (*(m_list.begin() + n)); 
		}

		void copy(tree<T>& _tree) const
		{
			_tree.clear();
			_tree.m_value = m_value;
			_tree.m_parent_ptr = NULL;		//. * 루트로 만든다.

			const_iterator iter = begin();
			for(; iter != end(); iter++)
				_tree.push_back((*iter));
		}

		pointer find(const T& value)
		{
			pointer _ptr = NULL;
			iterator iter = m_list.begin();
			for(; iter != m_list.end(); iter++) {
				if(memcmp(&((*iter).value()), &value, sizeof(T)) == 0)
					return (&(*iter));
				if(_ptr = (*iter).find(value))
					break;
			}
			return _ptr;
		}
		const_pointer find(const T& value) const
		{
			const_pointer _cptr = NULL;
			const_iterator iter = m_list.begin();
			for(; iter != m_list.end(); iter++) {
				if(memcmp(&((*iter).value()), &value, sizeof(T)) == 0)
					return (&(*iter));
				if(_cptr = (*iter).find(value))
					break;
			}
			return _cptr;
		}
		
		template<class _Td,class _Ts>
		pointer find_with_member(_Td _Ts::* m, const _Td& value)
		{
			pointer _ptr = NULL;
			iterator iter = m_list.begin();
			for(; iter != m_list.end(); iter++) {
				if(mem_var(m)((*iter).value()) == value)
					return (&(*iter));
				if(_ptr = (*iter).find_with_member(m, value))
					break;
			}
			return _ptr;
		}
		template<class _Td,class _Ts>
		const_pointer find_with_member(_Td _Ts::* m, const _Td& value) const
		{
			const_pointer _ptr = NULL;
			const_iterator iter = m_list.begin();
			for(; iter != m_list.end(); iter++) {
				if(mem_var(m)((*iter).value()) == value)
					return (&(*iter));
				if(_ptr = (*iter).find_with_member(m, value))
					break;
			}
			return _ptr;
		}
		
		reference operator = (const tree<T>& _tree)		//. 중간노드를 루트로 복사할때는 copy를 사용할것!
		{
			clear();
			m_parent_ptr = _tree.m_parent_ptr;		//. *
			m_value = _tree.m_value;

			const_iterator iter = _tree.begin();
			for(; iter != _tree.end(); iter++)
				push_back((*iter));
			return (*this);
		}
		reference operator = (const T& _value)
		{
			m_value = _value; 
			return (*this);
		}
		reference operator [] (size_t n) 
		{ return at(n); }
		const_reference operator [] (size_t n) const 
		{ return at(n); }

	private:
		T	m_value;
		tree_type*	m_parent_ptr;
		std::deque<tree_type>	m_list;
		allocator	m_allocator;
	};

};
#endif // _STLUTL_H_