#include "mimeparser.h"
#include <stdio.h>
#include <vector>

MIMEParser::MIMEParser() {
	//
}

int MIMEParser::ParseMessage(char * message, int messageSize) {
	MIMEPart * mp;
	MIMEPart * ch;
	
	mp = new MIMEPart(message, messageSize);
	mp->ParseHeader();
	if (mp->Tokenise("Content-type") != 0) {
		delete mp;
		return -1;
	}
	mp->FindSectionEnd();
	messageParts.push_back(mp);
	ch = mp;
	while (ch->MoreSections() == 1) {
		ch = new MIMEPart(mp, ch->GetSectionEnd(), ch->GetSectionSize());
		ch->ParseHeader();
		ch->Tokenise("Content-Type");
		ch->FindSectionEnd();
		messageParts.push_back(ch);
	}
	return 0;
}

int MIMEParser::GetNumberOfParts() {
	return messageParts.size();
}
