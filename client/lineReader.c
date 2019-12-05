#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../common/utils.h"
#include "lineReader.h"

/** @brief Acquires a line from the stdin and returns it.
 * @return a string containing the contents of the input.
 */
char* getLine() {
	int bufferSize = FILENAME_MAX;
	int pos = 0;
	char *line = malloc(bufferSize);
	int c;

	if (!line) 
        FAIL_SUCCESFULLY("mAllocation Error");

	while (1) {
		c = getchar();	// Get a character.
		if (c == EOF || c == '\n') {		// Acquire end of line (Enter key)
			line[pos] = '\0';
			return line;
		} else
			line[pos] = c;	// Append to buffer
		
		if (pos >= bufferSize) {	// Reallocate if necessary
			bufferSize += FILENAME_MAX;
			line = realloc(line, bufferSize);
			if (!line) FAIL_SUCCESFULLY("mAllocation Error");
		}
		pos++;
	}
}

#define LINE_SPLIT_PLACES " \t"

/** @brief Tokenizes the user's input
 * @param line The users input.
 * @param count The amount of params, useful externally.
 * @return The different tokens.
 */
char **splitLine(char *line, int *count) {
	int c = 0;
	for (int i = 0; i < strlen(line); i++) // Count the number of spaces to define the size of our mock _argv.
		if (line[i] == ' ' && line[i + 1] && line[i + 1] != ' ')
			c += 1;
			
	int pos = 0;
	char **elements = malloc((c + 2) * sizeof(char*));		// Token holder
	memset(elements, 0, (c + 2) * sizeof(char*));
	char *element;

	if (!elements) 
        FAIL_SUCCESFULLY("mAllocation Error");	// Handle errors

	element = strtok(line, LINE_SPLIT_PLACES);	// Tokenize on flags
	while (element != NULL) {	// Iterate on tokens
		elements[pos] = (char*) malloc(FILENAME_MAX);
		memset(elements[pos], 0, FILENAME_MAX);
		strncpy(elements[pos], element, FILENAME_MAX);
		pos++;
		element = strtok(NULL, LINE_SPLIT_PLACES);
	}

	elements[pos] = NULL;

	*count = pos;	// External use
	return elements;
}