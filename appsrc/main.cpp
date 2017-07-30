#include "mimepart.h"
#include "mimeparser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>
#include <vector>

int main(int argc, char *argv[]) {
	char * mimeBuffer;
	char * thingyBuffer;
	char * attr, * value, * nextToken;
	int res, emlSize;
	FILE * f;
	std::vector<std::pair<char *, char *>*> myList;
	
	if (argc != 2) {
		fprintf(stderr, "Usage: %s filetoparse\n", argv[0]);
		return 1;
	}

	mimeBuffer = (char *)malloc(65536 * 3);
	f = fopen (argv[1], "r");
	fread (mimeBuffer, 65500 * 3, 1, f);
	emlSize = ftell(f);
	mimeBuffer[emlSize] = '\0';
	fclose (f);
	
	fprintf (stdout, "Read %d bytes of email file\n", emlSize);
	
	MIMEParser * p = new MIMEParser();
	res = p->ParseMessage(mimeBuffer, emlSize);

	if (res == 0) {
		fprintf (stdout, "Parse (of top level) successful!\n");
		fprintf (stdout, "Number of top level parts in file %d\n", p->GetNumberOfParts());
	} else {
		fprintf (stdout, "Parse failed!\n");
	}

	free(mimeBuffer);
	return 0;
}

