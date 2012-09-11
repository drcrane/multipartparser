#ifndef __MIMEPART_H__
#define __MIMEPART_H__

#include <map>
#include <strings.h>

#define NUM_HEADERS 20
#define NUM_DECODED 20

class MIMEPart {
	public:
		MIMEPart(char * startAt, long size);
		MIMEPart(MIMEPart *parent, char * startAt, long size);
		~MIMEPart();
		int Parse();
		int ParseHeader();
		char *GetHeader(char *header);
		int GetHeaderPos(char * header);
		char *GetHeaderAttr(char * header, char * attr);
		char *GetBoundary();
		char *GetSectionEnd();
		long GetSectionSize();
		bool MoreSections();
		char *GetContent();
		MIMEPart *GetNextSection();
		int FindSectionEnd();
		int IsMultipart();
		int Tokenise(char * headerStr);
		int UtilTokenise (char * line, char **attr, char **value, char **nextToken);
	private:
		MIMEPart * parent;
		char *boundary;
		struct strCmp {
			bool operator()(char* s1, char* s2 ) {
				return strcasecmp( s1, s2 ) < 0;
			}
		} ;
		std::map<char *, char *, strCmp> *headers;
		std::map<char *, std::map<char *, char *, strCmp> *, strCmp> *tokenisedHeaders;
		char *headerStart;
		char *headerEnd;
		char *sectionEnd;
		char *contentBegin;
		char *contentEnd;
		bool moreSections;
		bool useParentBoundary;
		/* Size of Section (excluding headers) */
		long sectionSize;
		long noHeaders;
		long totalSize;
} ;

#endif

