/** @author Nicolas Boeckh */

#ifndef HEADER_CLIENT_LINEREADER
#define HEADER_CLIENT_LINEREADER

char* getLine();

char **splitLine(char *line, int *count);

#endif