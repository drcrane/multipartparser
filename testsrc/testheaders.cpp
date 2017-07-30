#include "mimepart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>
#include <vector>

int loadfile(char * filename, char ** emailbuffer, int * emailsize) {
	int res;
	size_t sz;
	char * buf = NULL;
	FILE * f = NULL;
	
	buf = (char *)malloc(65536 * 3);
	if (buf == NULL) { goto error; }
	f = fopen (filename, "r");
	if (f == NULL) { goto error; }
	res = fread (buf, (65536 * 3) - 1, 1, f);
	if (res != 0) { goto error; }
	sz = ftell(f);
	buf[sz] = '\0';
	fclose(f);
	f = NULL;

	*emailsize = sz;
	*emailbuffer = buf;

	return 0;
error:
	if (buf != NULL) {
		free(buf);
	}
	if (f != NULL) {
		fclose(f);
	}
	return -1;
}

int main(int argc, char *argv[]) {
	char * emailbuffer;
	char * thingyBuffer;
	char * attr, * value, * nextToken;
	char * subject;
	int res, emailsize;
	std::vector<std::pair<char *, char *>*> myList;
	std::vector<struct MIMEPart::attrval> * contenttype;
	std::vector<struct MIMEPart::attrval>::iterator it;
	MIMEPart * part = NULL;
	
	res = loadfile(argv[1], &emailbuffer, &emailsize);
	if (res != 0) {
		fprintf(stderr, "Could not load email \"%s\"\n", argv[1]);
		goto error;
	}
	fprintf (stdout, "Read %d bytes of email file\n", emailsize);
	
	part = new MIMEPart(emailbuffer, emailsize);
	res = part->ParseHeader();
	subject = part->GetHeader("Subject", 0);
	fprintf(stdout, "Subject: %s\n", subject);
	value = part->GetHeader("Content-Type", 0);
	fprintf(stdout, "Content-Type: %s\n", value);
	value = part->GetHeader("Content-type", 0);
	fprintf(stdout, "Content-type: %s\n", value);

	//contenttype = part->TokeniseHeader(value);
	//for (it = contenttype->begin(); it < contenttype->end(); it++) {
	//	fprintf(stdout, "%s is %s\n", (*it).attr, (*it).value);
	//}
	//delete contenttype;
	
	value = part->GetHeader("DomainKey-Signature", 0);
	//fprintf(stdout, "DomainKey-Signature %s\n", value);
	if (value != NULL) {
		contenttype = part->TokeniseHeader(value);
		for (it = contenttype->begin(); it < contenttype->end(); it++) {
			fprintf(stdout, "%s is %s\n", (*it).attr, (*it).value);
		}
	}

	part->FindSectionEnd();

	if (res != 0) {
		fprintf (stdout, "Parse failed!\n");
		goto error;
	}

	fprintf (stdout, "Parse (of top level) successful!\n");
	//fprintf (stdout, "Number of top level parts in file %d\n", p->GetNumberOfParts());

	// print a header...
	attr = "Received";
	

	free(emailbuffer);
	return 0;
error:
	if (part != NULL) {
		delete part;
	}
	if (emailbuffer != NULL) {
		free(emailbuffer);
	}
	return 1;
}

