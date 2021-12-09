#include "stdafx.h"

#include "xstreambuf.h"

using namespace leaf;

// xstreambuf

void xstreambuf::init() 
{
	m_pBuffer = NULL;
	m_offset = 0;
	m_size = m_capacity = 0;
	m_chunksize = 0;
}

xstreambuf::xstreambuf() 
{ init(); }
xstreambuf::xstreambuf(unsigned int size) 
{
	init();
	resize(size);
}
xstreambuf::xstreambuf(const xstreambuf& xbuf)
{
	init();
	write(xbuf.data(), xbuf.size());
}
xstreambuf::~xstreambuf()
{
	if(m_pBuffer) {
		delete [] m_pBuffer;
	}
}

void xstreambuf::seek(unsigned int n,XBUF_POS pos) 
{
	if(pos == XBUF_BEGIN) {
		m_offset = n ;
	}
	else if(pos == XBUF_END) {
		m_offset = size()-n;
	}
	if(m_offset < 0)
		m_offset = 0;
	if(m_offset > size())
		m_offset = size();
}
xstreambuf& xstreambuf::write(const void* src, unsigned int n) 
{
	unsigned int s = m_offset + n;
	if(size() < s )
		resize(s);
	
	memcpy((unsigned char*)(m_pBuffer)+m_offset, src, n);
	m_offset+=n;
	
	return *this;
}
xstreambuf& xstreambuf::read(void* dest, unsigned int n) 
{
	if(size() < m_offset + n) 
		n = (size() - 1) - m_offset;
	
	memcpy(dest,(const unsigned char*)(m_pBuffer)+m_offset, n);
	m_offset+=n;
	
	return *this;
}

void xstreambuf::clear()
{ 
	if(m_pBuffer) {
		delete [] m_pBuffer;
	}
	init();
}
void xstreambuf::flush()
{ m_offset = 0; }

const void* xstreambuf::data() const
{ return m_pBuffer; }
void* xstreambuf::get_writebuf() const
{ return m_pBuffer; }

unsigned int xstreambuf::get_curpos() const
{ return m_offset; }
size_t xstreambuf::size() const 
{ return m_size; }
void xstreambuf::resize(size_t s) 
{
	if(m_capacity < s) {
		unsigned int buf_size = m_chunksize + (unsigned int)((float)s/4+0.99f)*4;
		char* temp = new char[buf_size];
		memset(temp,0,buf_size);
		if(m_pBuffer) {
			unsigned int cpysize = 0;
			if(s < size())
				cpysize = s;
			else
				cpysize = size();
			
			memcpy(temp, m_pBuffer, cpysize);
			delete [] m_pBuffer;
		}
		m_pBuffer = temp;
		m_capacity = buf_size;
	}
	m_size = s;
	if(m_offset > m_size) 
		m_offset = size();
}
size_t xstreambuf::capacity() const 
{ return m_capacity; }
bool xstreambuf::empty() const 
{ return m_size == 0; }

void xstreambuf::set_chunksize(size_t size)
{ m_chunksize = size; }
size_t xstreambuf::get_chunksize()
{ return m_chunksize; }


// xfstreambuf

xfstreambuf::xfstreambuf() 
{}
xfstreambuf::xfstreambuf(const xstreambuf& xbuf)
{
	write(xbuf.data(), xbuf.size());
}
xfstreambuf::xfstreambuf(const std::string& filename)
{
	load(filename);
}
xfstreambuf::~xfstreambuf()
{}

bool xfstreambuf::load(const std::string& filename) 
{
	FILE* fp = NULL;
	fp = fopen(filename.data(), "rb");
	if(fp) {
		fseek(fp, 0, SEEK_END);
		unsigned int size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		
		char* temp = new char[size];
		fread(temp, 1, size, fp);
		write(temp, size);
		delete [] temp;
		
		fclose(fp);
		
		return true;
	}
	return false;
}
bool xfstreambuf::save(const std::string& filename) 
{
	FILE* fp = NULL;
	fp = fopen(filename.data(), "wb");
	if(fp) {
		fwrite(data(), 1, size(), fp);
		fclose(fp);
		
		return true;
	}
	return false;
}
