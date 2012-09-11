#ifndef __MIMEPARSER_H__
#define __MIMEPARSER_H__

#include <vector>
#include "mimepart.h"

class MIMEParser {
public:
	MIMEParser();
	int ParseMessage(char * message, int messageSize);
	int GetNumberOfParts();
private:
	std::vector<MIMEPart *> messageParts;
} ;

#endif
