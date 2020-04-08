#include "mimepart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>
#include <vector>

int loadfile(char * filename, char ** emailbuffer, size_t * emailsize) {
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
	size_t emailsize;
	int res;

	MIMEPart * part = NULL;
	
	res = loadfile(argv[1], &emailbuffer, &emailsize);
	if (res != 0) {
		fprintf(stderr, "Could not load email \"%s\"\n", argv[1]);
		goto error;
	}
	fprintf (stdout, "Read %d bytes of email file\n", (int)emailsize);
	
	part = new MIMEPart(emailbuffer, emailsize);
	res = part->ParseHeader();

	fprintf(stdout, "Number of headers %d\n", res);
	
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

