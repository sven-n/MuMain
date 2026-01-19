#ifndef _XSTREAMBUF_H_
#define _XSTREAMBUF_H_

#pragma warning(disable : 4786)
#include <string>

namespace leaf {
    class xstreambuf {
        void* m_pBuffer;
        unsigned int	m_offset;
        size_t	m_size, m_capacity;
        size_t	m_chunksize;

        void init();
    public:
        enum XBUF_POS { XBUF_BEGIN, XBUF_END };

        xstreambuf();
        xstreambuf(unsigned int size);
        xstreambuf(const xstreambuf& xbuf);
        virtual ~xstreambuf();

        void seek(unsigned int n, XBUF_POS pos = XBUF_BEGIN);
        xstreambuf& write(const void* src, unsigned int n);
        xstreambuf& read(void* dest, unsigned int n);

        void clear();
        void flush();

        const void* data() const;
        void* get_writebuf() const;		/* Don't use as possible */

        unsigned int get_curpos() const;	/* Gets the current position */
        size_t size() const;
        void resize(size_t s);
        size_t capacity() const;
        bool empty() const;

        void set_chunksize(size_t size);
        size_t get_chunksize();

        template<class T> xstreambuf& operator << (const T& data)
        {
            return write((void*)(&data), sizeof(data));
        }

        xstreambuf& operator << (char* sz)
        {
            return write(sz, strlen(sz) + 1);
        }
        xstreambuf& operator << (const char* sz)
        {
            return write(sz, strlen(sz) + 1);
        }
        xstreambuf& operator << (std::string str)
        {
            return write(str.data(), str.size() + 1);
        }

        template<class T> xstreambuf& operator >> (T& data)
        {
            return read((char*)&data, sizeof(data));
        }

        xstreambuf& operator >> (char* sz)
        {
            return read(sz, strlen((const char*)(m_pBuffer)+m_offset) + 1);
        }
        xstreambuf& operator >> (std::string str)
        {
            str = (const char*)(m_pBuffer)+m_offset;
            m_offset += str.length();
            return *this;
        }

        xstreambuf& operator = (const xstreambuf& buf)
        {
            if (size() < buf.size())
                resize(buf.size());

            memcpy(m_pBuffer, buf.data(), buf.size());

            m_size = buf.size();
            m_offset = size();
            return *this;
        }
    };

}

#endif // _XSTREAMBUF_H_