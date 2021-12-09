#include "stdafx.h"

#include "html_log.h"
#include "stdleaf.h"

using namespace leaf;


CHtmlLog::CHtmlLog()
{}
CHtmlLog::~CHtmlLog()
{}

bool CHtmlLog::LoadHtmlLog(const std::string& filename)
{
	FILE* fp = fopen(filename.c_str(), "rb");
	if(fp == NULL)
		return false;

	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	BYTE* pbyLog = new BYTE[size];
	fread(pbyLog, 1, size, fp);

	//. Parsing Html

	int iBeginSyntax = -1, iEndSyntax = -1, iBodyPos = 0;
	enum KEYWORD_TYPE {
		TYPE_NONE = 0,
		TYPE_BEGIN_BODY,
		TYPE_END_BODY
	} KeyWordType = TYPE_NONE;

	for(int offset=0; offset<(int)size; offset++) 
	{
		if(pbyLog[offset] == '<') {
			iBeginSyntax = offset;
			iEndSyntax = -1;
		}
		else if(pbyLog[offset] == '>') {

			iEndSyntax = offset;
			
			if(KeyWordType == TYPE_BEGIN_BODY) {
				m_head.assign((const char*)pbyLog, iEndSyntax+1);
				iBodyPos = iEndSyntax+1;
				KeyWordType = TYPE_NONE;
			}
			else if(KeyWordType == TYPE_END_BODY) {
				if(pbyLog[iBeginSyntax-1] == '\n')
					iBeginSyntax--;
				if(pbyLog[iBeginSyntax-1] == '\r')
					iBeginSyntax--;

				m_body.assign((const char*)pbyLog+iBodyPos, iBeginSyntax-iBodyPos);
				m_tail.assign((const char*)pbyLog+iBeginSyntax, size-iBeginSyntax);
				break;
			}
		}
		else if(iEndSyntax < 0 && KeyWordType == TYPE_NONE) {
			//. Scan body
			if(strnicmp((const char*)pbyLog+offset, "BODY", 4) == 0)
				KeyWordType = TYPE_BEGIN_BODY;
			else if(strnicmp((const char*)pbyLog+offset, "/BODY", 5) == 0)
				KeyWordType = TYPE_END_BODY;
		}
	}

	delete [] pbyLog;

	fclose(fp);
	return true;
}
bool CHtmlLog::SaveHtmlLog(const std::string& filename)
{
	FILE* fp = fopen(filename.c_str(), "wb");
	if(fp == NULL)
		return false;
	fwrite(m_head.data(), 1, m_head.size(), fp);
	fwrite(m_body.data(), 1, m_body.size(), fp);
	fwrite(m_tail.data(), 1, m_tail.size(), fp);
	
	fclose(fp);

	return true;
}
void CHtmlLog::ClearHtmlLog()
{
	m_head = m_body = m_tail = std::string("");
}

void CHtmlLog::WriteBeginHtml(const std::string& title)
{
	m_head = "<HTML>\r\n";
	m_head += "<HEAD>\r\n";
	m_head += leaf::FormatString("<TITLE>%s</TITLE>\r\n", title.c_str());
	m_head += "</HEAD>\r\n";
}
void CHtmlLog::WriteEndHtml()
{
	m_tail += "\r\n</HTML>\r\n";
}
void CHtmlLog::WriteBeginBody(int bgcolor, const char* opt_add, ...)
{
	m_head += "\r\n<BODY ";
	if(bgcolor >= 0)
		m_head += leaf::FormatString("bgcolor=#%000006X ", bgcolor);
	if(opt_add) {
		char szOption[1024];
		va_list vlist;
		va_start(vlist, opt_add);
		vsprintf(szOption, opt_add, vlist);
		va_end(vlist);
		m_head += szOption;
	}
	m_head += ">\r\n";
}
void CHtmlLog::WriteEndBody()
{
	m_tail += "\r\n</BODY>";
}

void CHtmlLog::Write(const std::string& message)
{
	m_body += message;
}
void CHtmlLog::Write(const char* message, size_t size)
{
	m_body.append(message, size);
}

void CHtmlLog::WriteInsertNewLine()
{
	m_body += "<BR>\r\n";
}
void CHtmlLog::WriteInsertSpace(int count)
{
	for(int i=0; i<count; i++)
		m_body += "&nbsp;";
}
void CHtmlLog::WriteInsertImage(const std::string& img, const char* opt_add, ...)
{
	m_body += leaf::FormatString("<IMG src=\"%s\"", img.c_str());
	if(opt_add) {
		char szOption[1024];
		va_list vlist;
		va_start(vlist, opt_add);
		vsprintf(szOption, opt_add, vlist);
		va_end(vlist);
		m_body += szOption;
	}
	m_body += ">\r\n";
}
void CHtmlLog::WriteBeginSpan(const char* fontface, int font_pt, int bgcolor, int color)
{
	m_body += "<SPAN style=";
	if(fontface)
		m_body += leaf::FormatString("font-family:%s;", fontface);
	if(font_pt > 0)
		m_body += leaf::FormatString("font-size:%dpt;", font_pt);
	if(bgcolor >= 0)
		m_body += leaf::FormatString("background-color=%000006X;", bgcolor);
	if(color >= 0)
		m_body += leaf::FormatString("color=%000006X;",color);
	m_body += ">\r\n";
}
void CHtmlLog::WriteEndSpan()
{
	m_body += "\r\n</SPAN>\r\n";
}
void CHtmlLog::WriteBeginFont(const char* face, int size, int color)
{
	m_body += "<FONT ";
	if(face)
		m_body += leaf::FormatString("face=%s ", face);
	if(size > 0)
		m_body += leaf::FormatString("size=%d ", size);
	if(color >= 0)
		m_body += leaf::FormatString("color=%d ", color);
	m_body += ">\r\n";
}
void CHtmlLog::WriteEndFont()
{
	m_body += "\r\n</FONT>\r\n";
}
void CHtmlLog::WriteBeginHyperLink(const std::string& link, const std::string& target)
{
	m_body += leaf::FormatString("<A HREF = \"%s\" target=\"%s\">", link.c_str(), target.c_str());
}
void CHtmlLog::WriteEndHyperLink()
{
	m_body += "</A>\r\n";
}