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
	this->totalSize = size;
}

MIMEPart::~MIMEPart() {
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
	struct header currheader;
	int pos = 0;
	int i;
	char *start;
	char *value;
	char *val_end;
	char *header = headerStart;
	int currenthdr = 0;
	
	currheader.parsed = NULL;
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
	currheader.attr = start;
	currheader.value = value;
	headers.push_back(currheader);
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
	//this->noHeaders = currenthdr;
	//contentBegin = headerEnd;
	return currenthdr;
}

int MIMEPart::IterateHeaders() {
	std::vector<struct header>::iterator it;
	for (it = headers.begin(); it < headers.end(); it++) {
		fprintf(stdout, "H: %s\n", (*it).attr);
	}
	return 0;
}

char * MIMEPart::GetHeader(const char * attribute, int idx) {
	std::vector<struct header>::iterator it;
	for (it = headers.begin(); it < headers.end(); it++) {
		if (strcasecmp((*it).attr, attribute) == 0) {
			return (*it).value;
		}
	}
	return NULL;
}

/*
    Perform in-place tokenisation, this means that the char buffer
    given to this function will be mutated.
*/
std::vector<struct MIMEPart::attrval> * MIMEPart::TokeniseHeader(char * line) {
	char *tokenStart, *tokenEnd, *nextToken = NULL;
	char *eqPos, *coPos, *co2Pos;
	std::vector<struct attrval> * tokList;
	struct attrval av;
	nextToken = line;
	tokList = new std::vector<struct attrval>();
tryAgain:
	if (*nextToken == '\0') {
		goto finished;
	}
	//if (*nextToken == '\r' || *nextToken == '\n') {
	//	fprintf(stderr, "We got a line terminator\n");
	//	goto finished;
	//}
	if (*nextToken == ';' || *nextToken == ',') {
		nextToken++;
	}
	tokenStart = skip_allwhitespace(nextToken);
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
		av.value = tokenStart;
		av.attr = tokenStart;
		nextToken = coPos + 1;
		if (strlen(tokenStart) == 0) {
			
			goto finished;
		}
	} else {
		tokenEnd = strchr (tokenStart, '=');
		if (tokenEnd == NULL) {
			// The token does not exist from tokenStart...
			// This is probably malformed, ABORT!
			goto finished;
		}
		*tokenEnd = '\0';
		if (*(tokenEnd + 1) == '\"') {
			tokenEnd += 2;
			av.value = tokenEnd;
			while (*tokenEnd != '\"' && *tokenEnd != '\0') {
				if (*tokenEnd == '\\') {
					tokenEnd ++;
				}
				tokenEnd++;
			}
			if (*tokenEnd == '\"') {
				*tokenEnd = '\0';
			}
			nextToken = tokenEnd + 1;
			av.attr = tokenStart;
		} else {
			tokenEnd++;
			av.value = tokenEnd;
			while (*tokenEnd != '\r' && *tokenEnd != '\n' && *tokenEnd != '\t' && *tokenEnd != ' ' && *tokenEnd != ';' && *tokenEnd != ',' && *tokenEnd != '\0') {
				tokenEnd ++;
			}
			*tokenEnd = '\0';
			av.attr = tokenStart;
			nextToken = tokenEnd + 1;
		}
	}
	fprintf(stderr, "[%s]\n", av.attr);
	tokList->push_back(av);
	goto tryAgain;
finished:
	return tokList;
}

char * MIMEPart::GetAttributeByName(const char * attr, std::vector<struct MIMEPart::attrval> * attrval_vect) {
	std::vector<struct MIMEPart::attrval>::iterator it;
	for (it = attrval_vect->begin(); it < attrval_vect->end(); it++) {
		char * curr_attr = (*it).attr;
		if (curr_attr != NULL && strcmp(attr, curr_attr) == 0) {
			return (*it).value;
		}
	}
	return NULL;
}

int MIMEPart::FindSectionEnd() {
	char * tmp, * offs;
	char * boundary;
	int d;
	std::vector<struct MIMEPart::attrval> * attrval_vect;
	int sectioncount = 0;
	int sectionsize;

	tmp = this->GetHeader("Content-type", 0);
	if (tmp == NULL) {
		return -1;
	}
	attrval_vect = this->TokeniseHeader(tmp);
	tmp = this->GetAttributeByName("boundary", attrval_vect);
	if (tmp == NULL) {
		return -1;
	}
	d = strlen(tmp);
	boundary = (char *)malloc(d+13);
	strcpy (boundary, "--");
	strcat (boundary, tmp);
	d += 2;

	fprintf(stdout, "the section end is %s\n", boundary);

	tmp = headerEnd;
retry:
	offs = strstr (tmp, boundary);
	if (offs == NULL) {
		free(boundary);
		return -1;
	}
	// The boundary must start after a new line, if not then it is not a boundary.
	if (*(offs - 1) != '\r' && *(offs - 1) != '\n') {
		tmp = offs + d;
		goto retry;
	}

	if (offs[d] == '-' && offs[d+1] == '-') {
		// This is the end of the section.
		sectionsize = offs - tmp;
		fprintf(stdout, "last Section %d size %d\n", sectioncount, sectionsize);
	} else {
		sectionsize = offs - tmp;
		fprintf(stdout, "Section %d size %d\n", sectioncount, sectionsize);
		sectioncount ++;
		tmp = offs + d;
		goto retry;
	}
	offs[d + 10] = '\0';
	fprintf(stdout, "%s\n", offs);

	fprintf(stdout, "number of sections: %d\n", sectioncount);
	return 0;
}



