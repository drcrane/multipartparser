#include "mimepart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>
#include <map>

/*
Sample base64 Line:
k7nb1Y1lKUpFJJEKRBugqZYgo4Aq6Lc+mBS+SB1pcrHcpFHPQ/lULQOx5JrR
*/

MIMEPart::MIMEPart(char * startAt, long size) {
	this->headerStart = startAt;
	this->parent = NULL;
	this->totalSize = size;
	tokenisedHeaders = NULL;
	this->useParentBoundary = false;
}

MIMEPart::MIMEPart(MIMEPart *parent, char * startAt, long size) {
	this->headerStart = startAt;
	this->parent = parent;
	this->totalSize = size;
	tokenisedHeaders = NULL;
	headers = NULL;
	this->useParentBoundary = true;
}

MIMEPart::~MIMEPart() {
	if (headers != NULL) {
		delete headers;
	}
	if (tokenisedHeaders != NULL) {
		delete tokenisedHeaders;
	}
}

static char *skip_whitespace(char * s) {
	while ( (*s == '\t' || *s == ' ') && *s != '\0' ) {
		s ++;
	}
	return s++;
}

static char *skip_allwhitespace(char * s) {
	while ( (*s == '\t' || *s == ' ' || *s == '\r' || *s == '\n') && *s != '\0' ) {
		s ++;
	}
	return s++;
}

static char *find_next_header(char * s) {
repeat:
	while ( !(*s == '\n' || *s == '\r') && *s != '\0' ) {
		s++;
	}
	if (*s == '\r' && *(s+1) == '\n') {
		s++;
	}
	s++;
	if (*s == '\t' || *s == ' ') {
		goto repeat;
	}
	return s;
}

int MIMEPart::Parse() {
	//
	return 0;
}

int MIMEPart::ParseHeader() {
	int pos = 0;
	int i;
	char *start;
	char *value;
	char *val_end;
	char *header = headerStart;
	int currenthdr = 0;
	
	/* using an STL C++ map to link the headername with the headervalue */
	std::map<char*, char *, strCmp> *headers = new std::map<char*, char *, strCmp>;
	this->headers = headers;
	i = 0;
	/* Find the start of the header attribute */
	while ( !((header[i] >= 'a' && header[i] <= 'z') || (header[i] >= 'A' && header[i] <= 'Z')) && header[i] != '\0')  {
		i++;
	}
	start = header + i;
startagain:
	value = strchr(start, ':');
	*value = '\0';
	value = skip_whitespace(value + 1);
	val_end = find_next_header(value);
	if (*(val_end - 2) == '\r') {
		*(val_end - 2) = '\0';
	}
	*(val_end - 1) = '\0';
	(*headers)[start] = value;
	currenthdr++;
	if (*val_end != '\0' && *val_end != '\r' && *val_end != '\n') {
		start = val_end;
		goto startagain;
	}
	if ((*val_end == '\r' && *(val_end+1) == '\n')) {
		/* header ends at val_end + 2 */
		headerEnd = val_end + 2;
	} else {
		/* header ends at val_end + 1 */
		headerEnd = val_end + 1;
	}
	this->noHeaders = currenthdr;
	contentBegin = headerEnd;
	return currenthdr;
}

char *MIMEPart::GetHeader(char * header) {
	int i;
	return (*headers)[header];
}

int MIMEPart::GetHeaderPos(char * header) {
	int i;
}

char *MIMEPart::GetHeaderAttr(char * header, char * attr) {
	if ((*tokenisedHeaders)[header] == NULL) {
		return NULL;
	}
	return (*(*tokenisedHeaders)[header])[attr];
}

char * MIMEPart::GetBoundary() {
	return boundary;
}

int MIMEPart::IsMultipart() {
	if (tokenisedHeaders == NULL) {
		return -1;
	}
	if ((*tokenisedHeaders)["Content-type"] == NULL) {
		/* ParseHeaders() and Tokenise("Content-Type") should be called first */
		return -1;
	}
	std::map<char*, char*, strCmp> * h;
	std::map<char*, char*, strCmp>::iterator it;
	h = (*tokenisedHeaders)["Content-type"];
	
	/*
	for ( it=(*h).begin() ; it != (*h).end(); it++ )
		fprintf(stdout, "%s = %s\n", (*it).first, (*it).second);
	*/
	if ((*h)["multipart/alternative"] != NULL || (*h)["multipart/mixed"] != NULL) {
		return 1;
	}
	return 0;
}

int MIMEPart::FindSectionEnd() {
	char * tmp, * offs;
	char * boundary;
	int d;
	
	if (parent != NULL && useParentBoundary) {
		tmp = parent->GetBoundary();
	} else {
		tmp = this->GetHeaderAttr("Content-type", "boundary");
	}
	//fprintf (stdout, "Boundary: %s\n", tmp);
	this->boundary = tmp;
	d = strlen(tmp);
	boundary = (char *)malloc(d+13);
	strcpy (boundary, "--");
	strcat (boundary, tmp);
	tmp = headerEnd;
retry:
	offs = strstr (tmp, boundary);
	if (offs == NULL) {
		free(boundary);
		return -1;
	}
	if (*(offs - 1) != '\r' && *(offs - 1) != '\n') {
		tmp = offs + d;
		goto retry;
	}
	free(boundary);
	contentEnd = offs;
	sectionEnd = offs + d + 2;
	if (*sectionEnd == '\r' && *(sectionEnd + 1) == '\n') {
		sectionEnd ++;
	}
	sectionEnd ++;
	sectionSize = contentEnd - headerEnd;
	if (*sectionEnd == '-' && *(sectionEnd - 1) == '-') {
		this->moreSections = false;
	} else {
		this->moreSections = true;
	}
	return 0;
}

char *MIMEPart::GetSectionEnd() {
	return sectionEnd;
}

MIMEPart *MIMEPart::GetNextSection() {
	//
}

bool MIMEPart::MoreSections() {
	return this->moreSections;
}

long MIMEPart::GetSectionSize() {
	return this->sectionSize;
}

char *MIMEPart::GetContent() {
	return headerEnd;
}

/*
    These functions take a header string and tokenise it so that it becomes a set of <attr,value> pairs.
    Often the first value does not have an attribute and so the first element is: attr=null val="value",
    an example is the Content-type header that often looks like this:
    "Content-type: multipart/alternative; charset=UTF8; boundary="crazy99silly762112"\r\n".
    These functions corrupt the buffer and so the buffer must be r/w and not required after tokenisation.
*/
int MIMEPart::Tokenise (char * headerStr) {
	char *attr = NULL, *value = NULL, *next = NULL;
	char *hdrContent;
	int i = 0;
	std::map<char *, char *, strCmp> *thisHeader = new std::map<char *, char *, strCmp>;
	
	hdrContent = (*headers)[headerStr];
	if (hdrContent == NULL) {
		return -1;
	}
	this->UtilTokenise(hdrContent, &attr, &value, &next);
	//fprintf (stdout, "%s.%s = %s\n", headerStr, attr, value);
	(*thisHeader)[attr] = value;
	while (this->UtilTokenise(next, &attr, &value, &next) == 0) {
		(*thisHeader)[attr] = value;
		//fprintf (stdout, "%s.%s = %s\n", headerStr, attr, value);
	}
	if (tokenisedHeaders == NULL) {
		tokenisedHeaders = new std::map<char *, std::map<char *, char *, strCmp> *, strCmp>;
	}
	(*tokenisedHeaders)[headerStr] = thisHeader;
	/* we screwed this value up :-( */
	/* but don't worry, all screwed up values can be re-constituted. */
	(*headers).erase(headerStr);
	return i;
}

int MIMEPart::UtilTokenise (char * line, char **attr, char **value, char **nextToken) {
	char *tokenStart;
	char *tokenEnd;
	char *eqPos, *coPos, *co2Pos;
	
	if (*line == '\0') {
		return -1;
	}
	if (*line == ';' || *line == ',') {
		line ++;
	}
	tokenStart = skip_allwhitespace(line);
	eqPos = strchr(tokenStart, '=');
	coPos = strchr(tokenStart, ';');
	co2Pos = strchr(tokenStart, ',');
	if (coPos == NULL && co2Pos != NULL) {
		coPos = co2Pos;
	}
	if (co2Pos != NULL && co2Pos < coPos) {
		coPos = co2Pos;
	}
	if (coPos != NULL && eqPos != NULL && coPos < eqPos) {
		*coPos = '\0';
		*value = tokenStart;
		*attr = tokenStart;
		*nextToken = coPos + 1;
		if (strlen(tokenStart) == 0) {
			return -1;
		}
	} else {
		tokenEnd = strchr (tokenStart, '=');
		//fprintf (stdout, "[%s]\n", tokenEnd);
		*tokenEnd = '\0';
		if (*(tokenEnd + 1) == '\"') {
			tokenEnd += 2;
			*value = tokenEnd;
			while (*tokenEnd != '\"' && *tokenEnd != '\0') {
				if (*tokenEnd == '\\')
					tokenEnd ++;
				tokenEnd ++;
			}
			if (*tokenEnd == '\"') {
				*tokenEnd = '\0';
			}
			*nextToken = tokenEnd + 1;
			*attr = tokenStart;
		} else {
			tokenEnd++;
			*value = tokenEnd;
			while (*tokenEnd != '\r' && *tokenEnd != '\n' && *tokenEnd != '\t' && *tokenEnd != ' ' && *tokenEnd != ';' && *tokenEnd != ',' && *tokenEnd != '\0') {
				tokenEnd++;
			}
			*tokenEnd = '\0';
			*attr = tokenStart;
			*nextToken = tokenEnd + 1;
		}
	}
	return 0;
}



