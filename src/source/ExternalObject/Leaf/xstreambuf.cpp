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
{
    init();
}
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
    delete[] m_pBuffer;
}

void xstreambuf::seek(unsigned int n, XBUF_POS pos)
{
    if (pos == XBUF_BEGIN) {
        m_offset = n;
    }
    else if (pos == XBUF_END) {
        m_offset = size() - n;
    }
    if (m_offset < 0)
        m_offset = 0;
    if (m_offset > size())
        m_offset = size();
}
xstreambuf& xstreambuf::write(const void* src, unsigned int n)
{
    unsigned int s = m_offset + n;
    if (size() < s)
        resize(s);

    memcpy((BYTE*)(m_pBuffer)+m_offset, src, n);
    m_offset += n;

    return *this;
}
xstreambuf& xstreambuf::read(void* dest, unsigned int n)
{
    if (size() < m_offset + n)
        n = (size() - 1) - m_offset;

    memcpy(dest, (const BYTE*)(m_pBuffer)+m_offset, n);
    m_offset += n;

    return *this;
}

void xstreambuf::clear()
{
    delete[] m_pBuffer;

    init();
}
void xstreambuf::flush()
{
    m_offset = 0;
}

const void* xstreambuf::data() const
{
    return m_pBuffer;
}
void* xstreambuf::get_writebuf() const
{
    return m_pBuffer;
}

unsigned int xstreambuf::get_curpos() const
{
    return m_offset;
}
size_t xstreambuf::size() const
{
    return m_size;
}
void xstreambuf::resize(size_t s)
{
    if (m_capacity < s) {
        unsigned int buf_size = m_chunksize + (unsigned int)((float)s / 4 + 0.99f) * 4;
        BYTE* temp = new BYTE[buf_size];
        memset(temp, 0, buf_size);
        if (m_pBuffer) {
            unsigned int cpysize = 0;
            if (s < size())
                cpysize = s;
            else
                cpysize = size();

            memcpy(temp, m_pBuffer, cpysize);
            delete[] m_pBuffer;
        }
        m_pBuffer = temp;
        m_capacity = buf_size;
    }
    m_size = s;
    if (m_offset > m_size)
        m_offset = size();
}
size_t xstreambuf::capacity() const
{
    return m_capacity;
}
bool xstreambuf::empty() const
{
    return m_size == 0;
}

void xstreambuf::set_chunksize(size_t size)
{
    m_chunksize = size;
}
size_t xstreambuf::get_chunksize()
{
    return m_chunksize;
}
