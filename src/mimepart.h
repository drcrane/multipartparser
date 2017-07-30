#ifndef __MIMEPART_H__
#define __MIMEPART_H__

#include <vector>
#include <strings.h>

#define NUM_HEADERS 20
#define NUM_DECODED 20

class MIMEPart {
public:
	struct header {
		char * attr;
		char * value;
		std::vector<struct attrval> * parsed;
	};
	struct attrval {
		char * attr;
		char * value;
	};
	MIMEPart(char * startAt, long size);
	MIMEPart(MIMEPart *parent, char * startAt, long size);
	~MIMEPart();
	int Parse();
	int ParseHeader();
	int IterateHeaders();
	char * GetHeader(const char * attribute, int idx);
	std::vector<struct attrval> * TokeniseHeader(char * line);
	char * GetAttributeByName(const char * attr, std::vector<struct attrval> * attrval_vect);
	int FindSectionEnd();
private:
	MIMEPart * parent;
	char *boundary;
	struct strCmp {
		bool operator()(char* s1, char* s2 ) {
			return strcasecmp( s1, s2 ) < 0;
		}
	} ;
	//std::map<char *, char *, strCmp> *headers;
	//std::map<char *, std::map<char *, char *, strCmp> *, strCmp> *tokenisedHeaders;
	std::vector<struct header> headers;
	char *headerStart;
	char *headerEnd;
	char *sectionEnd;
	long totalSize;
};
#endif

