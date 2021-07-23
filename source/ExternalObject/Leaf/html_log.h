
#ifndef _HTML_LOG_H_
#define _HTML_LOG_H_

#pragma once

#pragma warning(disable : 4786)
#include <string>

namespace leaf {

	class CHtmlLog {
		std::string m_head;
		std::string	m_body;
		std::string m_tail;

	public:
		CHtmlLog();
		virtual ~CHtmlLog();

		bool LoadHtmlLog(const std::string& filename);
		bool SaveHtmlLog(const std::string& filename);
		void ClearHtmlLog();

		//. Head and Tail
		void WriteBeginHtml(const std::string& title);
		void WriteEndHtml();
		void WriteBeginBody(int bgcolor = -1, const char* opt_add = NULL, ...);
		void WriteEndBody();

		//. Body
		void Write(const std::string& message);
		void Write(const char* message, size_t size);
		
		void WriteInsertNewLine();
		void WriteInsertSpace(int count = 1);
		void WriteInsertImage(const std::string& img, const char* opt_add = NULL, ...);
		void WriteBeginSpan(const char* fontface = NULL, int font_pt = -1, int bgcolor = -1, int color = -1);
		void WriteEndSpan();
		void WriteBeginFont(const char* face = NULL, int size = -1, int color = -1);
		void WriteEndFont();
		void WriteBeginHyperLink(const std::string& link, const std::string& target = "_blank");
		void WriteEndHyperLink();	
	};
}

#endif // _HTML_LOG_H_